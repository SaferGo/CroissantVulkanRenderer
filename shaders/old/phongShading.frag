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

   for (int i = 0; i < ubo.lightsCount; i++)
   {
      vec3 lightDir = normalize(
            vec3(ubo.lightPositions[i]) - inPosition
      );

      finalColor += calculatePhongLighthing(
            lightDir,
            vec3(ubo.lightColors[i]),
            inNormal,
            texel,
            inPosition,
            vec3(ubo.cameraPos)
      );
   }

   outColor = clamp(vec4(finalColor, 1.0), 0.0, 1.0);
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
