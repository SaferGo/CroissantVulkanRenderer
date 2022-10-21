#version 450

layout(std140, binding = 0) uniform UniformBufferObject
{
   mat4 model;
   mat4 view;
   mat4 proj;
   vec4 lightPositions[10];
   vec4 lightColors[10];
   vec4 cameraPos;
   int  lightsCount;
} ubo;

layout(binding = 1) uniform sampler2D baseColorSampler;
layout(binding = 2) uniform sampler2D metallicRoughnessSampler;
layout(binding = 3) uniform sampler2D normalSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

//////////////////////////////////////PBR//////////////////////////////////////
float normalDistribution(vec3 normalD, vec3 halfwayD, float a);
float geometry(vec3 normalD, vec3 viewD, vec3 lightD, float k);
vec3 fresnel(vec3 halfwayD, vec3 viewD, vec3 F0);
///////////////////////////////////////////////////////////////////////////////

vec3 calculateDirLight();
vec3 calculateSpotLight();

void main()
{
   mat3 TBN = transpose(mat3(inTangent, inBitangent, inNormal));

   vec3 normal = normalize(
         TBN * (texture(normalSampler, inTexCoord).rgb * 2.0 - 1.0)
   );

   vec3 albedo = vec3(texture(baseColorSampler, inTexCoord));
   float metallic = texture(metallicRoughnessSampler, inTexCoord).r;
   float roughness = texture(metallicRoughnessSampler, inTexCoord).g;

   vec3 ambient = vec3(0.5) * albedo;
   vec3 Lo = calculateDirLight();
   Lo += calculateSpotLight();

   vec3 color = ambient + Lo;

   outColor = vec4(color, 1.0);
}

vec3 calculateDirLight()
{
   mat3 TBN = transpose(mat3(inTangent, inBitangent, inNormal));

   vec3 normal = normalize(
         TBN * (texture(normalSampler, inTexCoord).rgb * 2.0 - 1.0)
   );

   vec3 albedo = vec3(texture(baseColorSampler, inTexCoord));
   float metallic = texture(metallicRoughnessSampler, inTexCoord).r;
   float roughness = texture(metallicRoughnessSampler, inTexCoord).g;

   vec3 F0 = vec3(0.04);
   F0 = mix(F0, albedo, metallic);
   vec3 view = normalize(vec3(ubo.cameraPos)- inPosition);
   
   // reflectance equation
   vec3 Lo = vec3(0.0);
   for (int i = 0; i < ubo.lightsCount; i++)
   {
     vec3 lightDir = vec3(normalize(vec3(ubo.lightPositions[i]) - inPosition));
     vec3 halfwayDir = vec3(normalize(view + lightDir));

     float nDotV = max(dot(normal, view), 0.0);
     float nDotL = max(dot(normal, lightDir), 0.0);
     vec3 radiance = ubo.lightColors[i].rgb;

     // Cook-torrance brdf
     float NDF = normalDistribution(normal, halfwayDir, roughness);
     float G = geometry(normal, view, lightDir, roughness);
     vec3 F = fresnel(halfwayDir, view, F0);

     // Specular and Diffuse
     vec3 kS = F;
     vec3 kD = vec3(1.0) - kS;
     kD *= 1.0 - metallic;

     vec3 numerator = NDF * G * F;
     float denominator = (
           4.0 *
           nDotV *
           nDotL
     );
     vec3 specular = numerator / max(denominator, 0.0001);

     // Lo -> radiance
     Lo += (kD * (albedo / PI) + specular) * radiance * nDotL;
   }

   return Lo;
}

vec3 calculateSpotLight()
{
   mat3 TBN = transpose(mat3(inTangent, inBitangent, inNormal));

   vec3 normal = normalize(
         TBN * (texture(normalSampler, inTexCoord).rgb * 2.0 - 1.0)
   );

   vec3 albedo = vec3(texture(baseColorSampler, inTexCoord));
   float metallic = texture(metallicRoughnessSampler, inTexCoord).r;
   float roughness = texture(metallicRoughnessSampler, inTexCoord).g;

   vec3 F0 = vec3(0.04);
   F0 = mix(F0, albedo, metallic);
   vec3 view = normalize(vec3(ubo.cameraPos)- inPosition);
   
   // reflectance equation
   vec3 Lo = vec3(0.0);
   for (int i = 0; i < ubo.lightsCount; i++)
   {
     vec3 lightDir = vec3(normalize(vec3(ubo.lightPositions[i]) - inPosition));
     vec3 halfwayDir = vec3(normalize(view + lightDir));

     float nDotV = max(dot(normal, view), 0.0);
     float nDotL = max(dot(normal, lightDir), 0.0);
     vec3 color = ubo.lightColors[i].rgb * 100.0;
     float radius = 100.0;
     float distance = length(vec3(ubo.lightPositions[i]) - inPosition);
    float attenuation = pow(clamp(1 - pow((distance / radius), 4.0), 0.0, 1.0), 2.0)/(1.0  + (distance * distance) );
    vec3 radianceIn = color * attenuation;

     // Cook-torrance brdf
     float NDF = normalDistribution(normal, halfwayDir, roughness);
     float G = geometry(normal, view, lightDir, roughness);
     vec3 F = fresnel(halfwayDir, view, F0);

     // Specular and Diffuse
     vec3 kS = F;
     vec3 kD = vec3(1.0) - kS;
     kD *= 1.0 - metallic;

     vec3 numerator = NDF * G * F;
     float denominator = (
           4.0 *
           nDotV *
           nDotL
     );
     vec3 specular = numerator / max(denominator, 0.0000001);

     // Lo -> radiance
     Lo += (kD * (albedo / PI) + specular) * radianceIn * nDotL;
   }

   return Lo;
}


///////////////////////////////PBR - Helper functions//////////////////////////

/*
 * Trowbridge-Reitz GGX approximation.
 * 
 * Param. a: Surface's roughness.
 *
 */
float normalDistribution(vec3 normalD, vec3 halfwayD, float a)
{
   float a2 = a * a;
   float NdotH = max(dot(normalD, halfwayD), 0.0);
   float NdotH2 = NdotH * NdotH;

   float nominator = a2;
   float denominator = NdotH2 * (a2 - 1.0) + 1.0;
   denominator = PI * denominator * denominator;

   return nominator / denominator;
}

/*
 * Smith's Schlick GGX approximation.
 *
 * Param. k: Remapping of 'a' depending if we are using direct light or IBL.
 */
float geometry(vec3 normalD, vec3 viewD, vec3 lightD, float k)
{
   // Cache data.
   float NdotV = max(dot(normalD, viewD), 0.0);
   float NdotL = max(dot(normalD, lightD), 0.0);

   // Here we use the Schlick GGX function.
   float geometryObstruction = NdotV / (NdotV * (1.0 - k) + k);
   float geometryShadowing = NdotL / (NdotL * (1.0 - k) + k);

   // Here we use the Smith's Schlick GGX function.
   return geometryObstruction * geometryShadowing;
}

/*
 * Fresnel Schlick approximation.
 */
vec3 fresnel(vec3 halfwayD, vec3 viewD, vec3 F0)
{
   return F0 + (1.0 - F0) * pow(1.0 - max(dot(halfwayD, viewD), 0.0), 5.0);
}
