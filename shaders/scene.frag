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
layout(binding = 4) uniform sampler2D   normalSampler;

layout(binding = 5) uniform samplerCube irradianceMapSampler;
layout(binding = 6) uniform sampler2D   shadowMapSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inShadowCoords;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);


//////////////////////////////////////PBR//////////////////////////////////////
float distributionGGX(float nDotH, float rough);
float geometrySchlickGGX(float nDotV, float rough);
float geometrySmith(float nDotV, float nDotL, float rough);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
///////////////////////////////////////////////////////////////////////////////

vec3 calculateNormal();
vec3 calculateDirLight(
      int i,
      vec3 normal,
      vec3 view,
      vec3 albedo,
      float metallic,
      float roughness,
      vec3 F0
);
vec3 calculatePointLight(
      int i,
      vec3 normal,
      vec3 view,
      vec3 albedo,
      float metallic,
      float roughness,
      vec3 F0
);
float filterPCF(vec4 shadowCoords);
float calculateShadow(vec4 shadowCoords, vec2 off);
vec3 uncharted2Tonemap(vec3 color);
vec4 tonemap(vec4 color);
vec4 SRGBtoLINEAR(vec4 srgb);

float exposure = 18.0;
vec3 ambient = vec3(0.25);

void main()
{

   float pp = texture(irradianceMapSampler, vec3(0.0)).r;
   vec3 normal = calculateNormal();
   vec3 view = normalize(vec3(ubo.cameraPos) - inPosition);
   vec3 albedo     = SRGBtoLINEAR(
         tonemap(texture(baseColorSampler, inTexCoord))
   ).rgb;
   float metallic  = texture(metallicRoughnessSampler, inTexCoord).r;
   float roughness = texture(metallicRoughnessSampler, inTexCoord).g;
   roughness = max(0.03, metallic);

   // Material specular
   vec3 F0 = vec3(0.04);
   F0 = mix(F0, albedo, metallic);

   vec3 Lo = vec3(0.0);
   for (int i = 0; i < ubo.lightsCount; i++)
   {
      if (lights[i].type == 0)
      {
         Lo += calculateDirLight(
               i,
               normal,
               view,
               albedo,
               metallic,
               roughness,
               F0
         );
      } else if(lights[i].type == 1)
      {
         Lo += calculatePointLight(
               i,
               normal,
               view,
               albedo,
               metallic,
               roughness,
               F0
         );
      } else
         Lo += vec3(0.0);
   }

   float shadow = filterPCF(inShadowCoords / inShadowCoords.w);

   vec3 color = ambient * albedo + (1.0 - shadow) * Lo;

   outColor = SRGBtoLINEAR(vec4(color, 1.0));
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
      vec3 normal,
      vec3 view,
      vec3 albedo,
      float metallic,
      float roughness,
      vec3 F0
) {
   vec3 lightDir = normalize(-vec3(lights[i].dir));
   vec3 halfway = normalize(view + lightDir);
   float nDotV = max(dot(normal, view), 0.0);
   float nDotL = max(dot(normal, lightDir), 0.0);
   float nDotH = max(dot(normal, halfway), 0.0);
   float hDotV = max(dot(halfway, view), 0.0);
   vec3 inRadiance = lights[i].intensity * lights[i].color.rbg;

   // Cook-torrance brdf
   vec3 F = fresnelSchlick(hDotV, F0);
   float D = distributionGGX(nDotH, roughness);
   float G = geometrySmith(nDotV, nDotL, roughness);

   // Specular and Diffuse
   vec3 kS = F;
   vec3 kD = vec3(1.0) - kS;
   kD *= 1.0 - metallic;

   vec3 numerator = D * G * F;
   float denominator = 4.0 * nDotV * nDotL;
   vec3 specular = numerator / max(denominator, 0.0001);

   vec3 Lo = (kD * (albedo / PI) + specular) * inRadiance * nDotL;

   return Lo;
}

vec3 calculatePointLight(
      int i,
      vec3 view,
      vec3 normal,
      vec3 albedo,
      float metallic,
      float roughness,
      vec3 F0
) {
   return vec3(0.0);
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
