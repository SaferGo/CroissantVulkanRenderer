#version 450

layout(std140, binding = 0) uniform UniformBufferObject
{
   mat4 model;
   mat4 view;
   mat4 proj;
   mat4 lightSpace;
   vec4 cameraPos;
   int  lightsCount;
   bool hasNormalMap;

} ubo;

struct Light
{
   vec4 pos;
   vec4 dir;
   vec4 color;
   float attenuation;
   float radius;
   float intensity;
   int type;
};

layout(std140, binding = 1) uniform Lights
{
   Light lights[10];
};

layout(binding = 2) uniform sampler2D   baseColorSampler;
layout(binding = 3) uniform sampler2D   metallicRoughnessSampler;
layout(binding = 4) uniform sampler2D   emissiveColorSampler;
layout(binding = 5) uniform sampler2D   AOsampler;
layout(binding = 6) uniform sampler2D   normalSampler;

// PBR Samplers
layout(binding = 7) uniform samplerCube envMapSampler;
layout(binding = 8) uniform samplerCube irradianceMapSampler;
layout(binding = 9) uniform sampler2D   BRDFlutSampler;

layout(binding = 10) uniform sampler2D   shadowMapSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inShadowCoords;

layout(location = 0) out vec4 outColor;

struct Material
{
   vec3 albedo;
   float metallicFactor;
   float roughnessFactor;
   vec3 emissiveColor;
   float AO;
};

struct PBRinfo
{
   // cos angle between normal and light direction.
	float NdotL;          
   // cos angle between normal and view direction.
	float NdotV;          
   // cos angle between normal and half vector.
	float NdotH;          
   // cos angle between view direction and half vector.
	float VdotH;
   // Roughness value, as authored by the model creator.
   float perceptualRoughness;
   // color contribution from diffuse lighting.
	vec3 diffuseColor;    
   // color contribution from specular lighting.
	vec3 specularColor;
};

struct IBLinfo
{
   vec3 diffuseLight;
   vec3 specularLight;
   vec3 brdf;
};

const float PI = 3.14159265359;


//////////////////////////////////////PBR//////////////////////////////////////
float distributionGGX(float nDotH, float rough);
float geometrySchlickGGX(float nDotV, float rough);
float geometrySmith(float nDotV, float nDotL, float rough);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
///////////////////////////////////////////////////////////////////////////////

vec3 calculateNormal();
vec3 calculateDirLight(int i, Material material, PBRinfo pbrInfo);
void calculatePointLight();
float filterPCF(vec4 shadowCoords);
float calculateShadow(vec4 shadowCoords, vec2 off);
vec3 uncharted2Tonemap(vec3 color);
vec4 tonemap(vec4 color);
vec4 SRGBtoLINEAR(vec4 srgb);
vec3 getIBLcontribution(PBRinfo pbrInfo, IBLinfo iblInfo);
float exposure = 10.0;
float ambient = 0.3;

void main()
{ 
   vec3 normal = calculateNormal();
   vec3 view = normalize(vec3(ubo.cameraPos) - inPosition);
   vec3 reflection = -normalize(reflect(view, normal));

   Material material;
   {
      material.albedo = texture(baseColorSampler, inTexCoord).rgb;
      
      material.metallicFactor = texture(
            metallicRoughnessSampler, inTexCoord
      ).b;
      material.roughnessFactor = texture(
            metallicRoughnessSampler, inTexCoord
      ).g;

      material.AO = texture(AOsampler, inTexCoord).r;
      material.AO = (material.AO < 0.01) ? 1.0 : material.AO;

      material.emissiveColor = texture(emissiveColorSampler, inTexCoord).rgb;
   }

   PBRinfo pbrInfo;
   {
      float F0 = 0.04;

	   pbrInfo.NdotV = max(dot(normal, view), 0.001);
      pbrInfo.diffuseColor = material.albedo.rgb * (vec3(1.0) - vec3(F0));
      pbrInfo.diffuseColor *= 1.0 - material.metallicFactor;
      pbrInfo.specularColor = mix(
            vec3(F0),
            material.albedo,
            material.metallicFactor
      );

      pbrInfo.perceptualRoughness = clamp(
            material.roughnessFactor,
            // Min Roughness
            0.04,
            1.0
      );
   }

   IBLinfo iblInfo;
   {
      // HDR textures are already linear
      iblInfo.diffuseLight = texture(
         irradianceMapSampler,
         normal
         // converts cubemap coords to Vulkan coordinate space
         //normal * vec3(-1.0, -1.0, 1.0)
      ).rgb;

      vec2 brdfSamplePoint = clamp(
            vec2(
               pbrInfo.NdotV,
               1.0 - pbrInfo.perceptualRoughness
            ),
            vec2(0.0),
            vec2(1.0)
      );

      float mipCount = float(textureQueryLevels(envMapSampler));
      float lod = pbrInfo.perceptualRoughness * mipCount;
      iblInfo.brdf = textureLod(
            BRDFlutSampler,
            brdfSamplePoint,
            0
      ).rgb;
      
      iblInfo.specularLight = textureLod(
            envMapSampler,
            reflection.xyz,
            lod
      ).rgb;
   }


   vec3 color = getIBLcontribution(pbrInfo, iblInfo);
   for (int i = 0; i < ubo.lightsCount; i++)
   {
      vec3 lightDir = normalize(-vec3(lights[i].dir));
      vec3 halfway = normalize(view + lightDir);

      // Fills the data left for PBR
      {
         pbrInfo.NdotL = max(dot(normal, lightDir), 0.0);
         pbrInfo.NdotH = max(dot(normal, halfway), 0.0);
         pbrInfo.VdotH = max(dot(halfway, view), 0.0);
      }

      if (lights[i].type == 0)
         color += calculateDirLight(i, material, pbrInfo);
      else if(lights[i].type == 1)
         color += 0;
      else
         color += vec3(0.0);

   }
   float shadow = filterPCF(inShadowCoords / inShadowCoords.w);
   //vec3 color = ambient * material.albedo + (1.0 - shadow) * Lo;

   // AO
   color = material.AO * color;

   // Emissive
   color = material.emissiveColor + color;

   outColor = ambient * vec4(color, 1.0);
}

vec3 getIBLcontribution(PBRinfo pbrInfo, IBLinfo iblInfo)
{

   vec3 diffuse = iblInfo.diffuseLight * pbrInfo.diffuseColor;
   vec3 specular = (
         iblInfo.specularLight *
         (pbrInfo.specularColor * iblInfo.brdf.x + iblInfo.brdf.y)
   );

   return diffuse + specular;
}

vec3 uncharted2Tonemap(vec3 color)
{
   float A = 0.15;
   float B = 0.50;
   float C = 0.10;
   float D = 0.20;
   float E = 0.02;
   float F = 0.30;
   float W = 11.2;

   return (
         (color * (A * color + C * B) + D * E) /
         (color * (A * color + B) + D * F)
   ) - E / F;
}

vec4 tonemap(vec4 color)
{
   vec3 newColor = uncharted2Tonemap(color.rgb * exposure);
   newColor *= (1.0f / uncharted2Tonemap(vec3(11.2f)));

   return vec4(
         pow(
            newColor,
            vec3(1.0f / 2.2)
         ),
         color.a
   );
}

vec4 SRGBtoLINEAR(vec4 srgb)
{
	vec3 linOut = pow(srgb.xyz,vec3(2.2));

	return vec4(linOut,srgb.w);;
}

vec3 calculateNormal()
{
   mat3 TBN = mat3(inTangent, inBitangent, inNormal);

   if (ubo.hasNormalMap)
   {
      return normalize(
            TBN * (texture(normalSampler, inTexCoord).rgb * 2.0 - 1.0)
      );
   } else
      return inNormal;
}

float filterPCF(vec4 shadowCoords)
{
   vec2 texelSize = textureSize(shadowMapSampler, 0);
   float scale = 1.5;
   float dx = scale * 1.0 / float(texelSize.x);
   float dy = scale * 1.0 / float(texelSize.y);

   float shadow = 0.0;
   int count = 0;
   int range = 1;

   for (int x = -range; x <= range; x++)
   {
      for (int y = -range; y <= range; y++)
      {
         shadow += calculateShadow(
               shadowCoords,
               vec2(dx * x, dy * y)
         );
         count++;
      }
   }

   return shadow / count;
}

float calculateShadow(vec4 shadowCoords, vec2 off)
{
   if (shadowCoords.z > -1.0 && shadowCoords.z < 1.0)
   {
      float closestDepth = texture(shadowMapSampler, shadowCoords.xy + off).r;
      float currentDepth = shadowCoords.z;

      if (closestDepth < currentDepth)
         return 1.0;
   }

   return 0.0;
}

vec3 calculateDirLight(
      int i,
      Material material,
      PBRinfo pbrInfo
) {
   vec3 inRadiance = lights[i].intensity * lights[i].color.rbg;

   // Cook-torrance brdf
   vec3 F = fresnelSchlick(pbrInfo.VdotH, pbrInfo.specularColor);
   float D = distributionGGX(pbrInfo.NdotH, material.roughnessFactor);
   float G = geometrySmith(
         pbrInfo.NdotV,
         pbrInfo.NdotL,
         material.roughnessFactor
   );

   // Specular and Diffuse
   vec3 kS = F;
   vec3 kD = vec3(1.0) - kS;
   kD *= 1.0 - material.metallicFactor;

   vec3 numerator = D * G * F;
   float denominator = 4.0 * pbrInfo.NdotV * pbrInfo.NdotL;
   vec3 specular = numerator / max(denominator, 0.0001);

   return (
         (kD * (material.albedo / PI) + pbrInfo.specularColor) *
         inRadiance * pbrInfo.NdotL
   );
}

void calculatePointLight()
{
}


///////////////////////////////PBR - Helper functions//////////////////////////

/*
 * Trowbridge-Reitz GGX approximation.
 */
float distributionGGX(float nDotH, float rough)
{
   float a = rough * rough;
   float a2 = a * a;

   float denominator = nDotH * nDotH * (a2 - 1.0) + 1.0;
   denominator = 1 / (PI * denominator * denominator);

   return a2 * denominator;
}

float geometrySchlickGGX(float nDotV, float rough)
{
   float r = (rough + 1.0);
   float k = r * r / 8.0;

   float geometryObstruction = nDotV;
   float geometryShadowing = 1 / (nDotV * (1.0 - k) + k);

   return geometryObstruction * geometryShadowing;
}

float geometrySmith(float nDotV, float nDotL, float rough)
{
   float ggx2 = geometrySchlickGGX(nDotV, rough);
   float ggx1 = geometrySchlickGGX(nDotL, rough);

   return ggx1 * ggx2;
}

/*
 * Fresnel Schlick approximation.
 */
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
   float val = 1.0 - cosTheta;

   return F0 + (1.0 - F0) * pow(val, 5.0);
}
