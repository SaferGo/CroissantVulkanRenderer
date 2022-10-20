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

void main()
{
   //vec3 albedo = vec3(texture(baseColorSampler, inTexCoord));
   //float metallic = texture(metallicRoughnessSampler, inTexCoord).r;
   //float roughness = texture(metallicRoughnessSampler, inTexCoord).g;
   //vec3 normal = calculateNormal();

   //vec3 F0 = vec3(0.04);
   //F0 = mix(F0, albedo, metallic);
   //vec3 view   = normalize(vec3(ubo.cameraPos - inPosition));
   //
   //// reflectance equation
   //vec3 Lo = vec3(0.0);
   //for (int i = 0; i < ubo.lightsCount; i++)
   //{
   //  vec3 lightDir = vec3(normalize(ubo.lightPositions[i] - inPosition));
   //  vec3 halfwayDir = vec3(normalize(view + lightDir));

   //  float distance = length(ubo.lightPositions[i] - inPosition);
   //  float attenuation = 1.0 / (distance * distance);
   //  vec3 radiance = ubo.lightColors[i] * attenuation;

   //  // Cook-torrance brdf
   //  float NDF = normalDistribution(normal, halfwayDir, roughness);
   //  float G = geometry(normal, view, lightDir, roughness);
   //  vec3 F = fresnel(halfwayDir, view, F0);

   //  vec3 kS = F;
   //  vec3 kD = vec3(1.0) - kS;
   //  kD *= 1.0 - metallic;

   //  vec3 numerator = NDF * G * F;
   //  float denominator = (
   //        4.0 *
   //        max(dot(normal, view), 0.0) *
   //        max(dot(normal, lightDir), 0.0) +
   //        0.0001
   //   );
   //  vec3 specular = numerator / denominator;

   //  // outgoing radiance Lo
   //  float NdotL = max(dot(normal, lightDir), 0.0);
   //  Lo += (kD * albedo / PI + specular) * radiance * NdotL;
   //   
   //}
   //vec3 ambient = vec3(0.03) * albedo;
   //vec3 color = ambient + Lo;

   //outColor = clamp(vec4(finalColor, 1.0), 0.0, 1.0);
   //outColor = vec4(color, 1.0);
   

   mat3 TBN = transpose(mat3(inTangent, inBitangent, inNormal));

   vec3 normal = normalize(
         texture(normalSampler, inTexCoord).rgb * 2.0 - 1.0
   );

   vec3 finalColor = vec3(0.0);
   for (int i = 0; i < ubo.lightsCount; i++)
   {
      vec3 lightDir = normalize(
            (TBN * vec3(ubo.lightPositions[i])) -
            (TBN * inPosition)
      );

      vec3 ambient = ubo.lightColors[i].rgb * 0.5;
      vec3 diffuse = (
         max(
               0.0,
               dot(normal, lightDir)
         ) * ubo.lightColors[i].rgb * 0.5
      );

      finalColor += (
            (ambient + diffuse) *
            texture(baseColorSampler, inTexCoord).rgb
      );
   }

   outColor = vec4(finalColor, 1.0);
}

///////////////////////////////Helper functions////////////////////////////////

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
