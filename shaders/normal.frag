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
//layout(binding = 2) uniform sampler2D specularSampler;
//layout(binding = 3) uniform sampler2D normalSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

//////////////////////////////////////PBR//////////////////////////////////////
vec3 calculatePhongLighthing(
      vec3 lightDir,
      vec3 lightColor,
      vec3 objNormal,
      vec3 objColor,
      vec3 objPos,
      vec3 cameraPos
);
float normalDistribution(vec3 normalD, vec3 halfwayD, float a);
float geometry(vec3 normalD, vec3 viewD, vec3 lightD, float k);
vec3 fresnel(vec3 halfwayD, vec3 viewD, vec3 F0);

void main()
{
   
   vec3 finalColor = vec3(0.0);
   vec3 texel = vec3(texture(baseColorSampler, inTexCoord));

   //for (int i = 0; i < ubo.lightsCount; i++)
   //{
   //   vec3 lightDir = normalize(
   //         vec3(ubo.lightPositions[i]) - inPosition
   //   );

   //   finalColor += calculatePhongLighthing(
   //         lightDir,
   //         vec3(ubo.lightColors[i]),
   //         inNormal,
   //         texel,
   //         inPosition,
   //         vec3(ubo.cameraPos)
   //   );
   //}

   //outColor = clamp(vec4(finalColor, 1.0), 0.0, 1.0);
   outColor = vec4(texel, 1.0);
}

vec3 calculatePhongLighthing(
      vec3 lightDir,
      vec3 lightColor,
      vec3 objNormal,
      vec3 objColor,
      vec3 objPos,
      vec3 cameraPos
) {

   vec3 ambient = lightColor * 0.5;
   vec3 diffuse = (
         max(0.0, dot(objNormal, lightDir)) *
         lightColor *
         0.5
   );

   // - Specular
   vec3 viewDir = normalize(cameraPos - objPos);
   // The reflect function needs the light direction to point FROM the light 
   // source.
   vec3 reflectDir = reflect(-lightDir, objNormal);
   
   vec3 specular = (
         pow(
            max(
               dot(viewDir, reflectDir),
               0.0
            ),
            // Don't make this value even!
            33
         ) *
         lightColor *
         0.5
   );

   return (ambient + diffuse + specular) * objColor;
}

///////////////////////////////Helper functions////////////////////////////////

/*
 * Here we use the Trowbridge-Reitz GGX approximation.
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
 * Here we use the Smith's Schlick GGX approximation.
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
 * Here we use the Fresnel Schlick approximation.
 */
vec3 fresnel(vec3 halfwayD, vec3 viewD, vec3 F0)
{
   return F0 + (1.0 - F0) * pow(1.0 - dot(halfwayD, viewD), 5.0);
}
