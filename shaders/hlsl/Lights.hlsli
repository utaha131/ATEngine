#ifndef _LIGHTS_HLSL_
#define _LIGHTS_HLSL_

struct Light {
    uint Type; // 0 = Directional, 1 = Point
    float4 PositionOrDirection;
    float Radius;
};

struct LightSample {
    float3 wi;
    float3 L;
    float pdf;
    float wi_distance;
};

LightSample GetLightSample(float3 position, Light light) {
    LightSample light_sample;
    if (light.Type == 0) {
        light_sample.wi = light.PositionOrDirection;
        light_sample.L = float3(1.0f, 1.0f, 1.0f);
        light_sample.pdf = 1.0f;
        light_sample.wi_distance = 10000.0f;
    } else {
        float3 direction = light.PositionOrDirection.xyz - position.xyz;
        float distance = length(direction);
        light_sample.wi = normalize(direction);
        // if (distance <= light.Radius) {
        //     light_sample.L = float3(1.0f, 1.0f, 1.0f) / (distance * distance);
        // } else {
        //     light_sample.L = float3(0.0f, 0.0f, 0.0f);
        // }
        light_sample.L = float3(1.0f, 1.0f, 1.0f) / (distance * distance + 0.0001f);
        //light_sample.L = float3(1.0f, 1.0f, 1.0f) * pow(saturate(1 - pow(distance / light.Radius, 4)), 2) / (distance * distance  + 1);
        light_sample.pdf = 1.0f;
        light_sample.wi_distance = distance;//min(light.Radius, distance);
    }
    return light_sample;
}

//Hardcoded Scene Lights for now.

#define LIGHT_COUNT 1//80

#define FirstHeight 0.5f
#define ThirdHeight 12.0f

static Light Lights[LIGHT_COUNT] = {
    // //Directional
    { 0, { 0.0f, 1.0f, 0.2f, 0.0f }, 0.0f },

    //{ 1, { 0.0f, FirstHeight, -0.3f, 1.0f }, 1.15f },
    //1st Level.
//     { 1, { 0.0f, FirstHeight, 0.0f, 1.0f }, 1.15f },
//     { 1, { 4.1f, FirstHeight, 0.0f, 0.0f }, 1.15f },
//     { 1, { -4.5f, FirstHeight, 0.0f, 0.0f }, 1.15f },
//     { 1, { 8.2f, FirstHeight, 0.0f, 1.0f }, 1.15f },
//     { 1, { -9.0f, FirstHeight, 0.0f, 1.0f }, 1.15f },

//     { 1, { 2.05f, FirstHeight, 0.0f, 0.0f }, 1.15f },
//     { 1, { -2.25f, FirstHeight, 0.0f, 0.0f }, 1.15f },
//     { 1, { 6.15f, FirstHeight, 0.0f, 1.0f }, 1.15f },
//     { 1, { -6.75f, FirstHeight, 0.0f, 1.0f }, 1.15f },


//     { 1, { 0.0f, FirstHeight, 3.2f, 1.0f }, 1.15f },
//     { 1, { 4.1f, FirstHeight, 3.2f, 0.0f }, 1.15f },
//     { 1, { -4.5f, FirstHeight, 3.2f, 0.0f }, 1.15f },
//     { 1, { 8.2f, FirstHeight, 3.2f, 1.0f }, 1.15f },
//     { 1, { -9.0f, FirstHeight, 3.2f, 1.0f }, 1.15f },

//     { 1, { 2.05f, FirstHeight, 3.2f, 0.0f }, 1.15f },
//     { 1, { -2.25f, FirstHeight, 3.2f, 0.0f }, 1.15f },
//     { 1, { 6.15f, FirstHeight, 3.2f, 1.0f }, 1.15f },
//     { 1, { -6.75f, FirstHeight, 3.2f, 1.0f }, 1.15f },

//     { 1, { 0.0f, FirstHeight, -3.2f, 1.0f }, 1.15f },
//     { 1, { 4.1f, FirstHeight, -3.2f, 0.0f }, 1.15f },
//     { 1, { -4.5f, FirstHeight, -3.2f, 0.0f }, 1.15f },
//     { 1, { 8.2f, FirstHeight, -3.2f, 1.0f }, 1.15f },
//     { 1, { -9.0f, FirstHeight, -3.2f, 1.0f }, 1.15f },

//     { 1, { 2.05f, FirstHeight, -3.2f, 0.0f }, 1.15f },
//     { 1, { -2.25f, FirstHeight, -3.2f, 0.0f }, 1.15f },
//     { 1, { 6.15f, FirstHeight, -3.2f, 1.0f }, 1.15f },
//     { 1, { -6.75f, FirstHeight, -3.2f, 1.0f }, 1.15f },


//     //2nd Level.
//     { 1, { 8.2f, 4.0f, 0.0f, 1.0f }, 1.15f },
//     { 1, { -9.0f, 4.0f, 0.0f, 1.0f }, 1.15f },

//     { 1, { 0.0f, 4.0f, 3.2f, 1.0f }, 1.15f },
//     { 1, { 4.1f, 4.0f, 3.2f, 1.0f }, 1.15f },
//     { 1, { -4.5f, 4.0f, 3.2f, 1.0f }, 1.15f },
//     { 1, { 0.0f, 4.0f, -3.2f, 1.0f }, 1.15f },
//     { 1, { 4.1f, 4.0f, -3.2f, 1.0f }, 1.15f },
//     { 1, { -4.5f, 4.0f, -3.2f, 1.0f }, 1.15f },

//     { 1, { 2.05f, 4.0f, 3.2f, 0.0f }, 1.15f },
//     { 1, { -2.25f, 4.0f, 3.2f, 0.0f }, 1.15f },
//     { 1, { 6.15f, 4.0f, 3.2f, 1.0f }, 1.15f },
//     { 1, { -6.75f, 4.0f, 3.2f, 1.0f }, 1.15f },

//     { 1, { 2.05f, 4.0f, -3.2f, 0.0f }, 1.15f },
//     { 1, { -2.25f, 4.0f, -3.2f, 0.0f }, 1.15f },
//     { 1, { 6.15f, 4.0f, -3.2f, 1.0f }, 1.15f },
//     { 1, { -6.75f, 4.0f, -3.2f, 1.0f }, 1.15f },

//     //3rd Level
//     { 1, { 0.0f, ThirdHeight, 3.2f, 1.0f }, 1.15f },
//     { 1, { 4.1f, ThirdHeight, 3.2f, 1.0f }, 1.15f },
//     { 1, { -4.5f, ThirdHeight, 3.2f, 1.0f }, 1.15f },
//     { 1, { 0.0f, ThirdHeight, -3.2f, 1.0f }, 1.15f },
//     { 1, { 4.1f, ThirdHeight, -3.2f, 1.0f }, 1.15f },
//     { 1, { -4.5f, ThirdHeight, -3.2f, 1.0f }, 1.15f },

//     { 1, { 2.05f, ThirdHeight, 3.2f, 0.0f }, 1.15f },
//     { 1, { -2.25f, ThirdHeight, 3.2f, 0.0f }, 1.15f },
//     { 1, { 6.15f, ThirdHeight, 3.2f, 1.0f }, 1.15f },
//     { 1, { -6.75f, ThirdHeight, 3.2f, 1.0f }, 1.15f },

//     { 1, { 2.05f, ThirdHeight, -3.2f, 0.0f }, 1.15f },
//     { 1, { -2.25f, ThirdHeight, -3.2f, 0.0f }, 1.15f },
//     { 1, { 6.15f, ThirdHeight, -3.2f, 1.0f }, 1.15f },
//     { 1, { -6.75f, ThirdHeight, -3.2f, 1.0f }, 1.15f },

//     //Outside
// #define OutSideHeight 5.0f
//     { 1, { 0.0f, OutSideHeight, 11.0f, 1.0f }, 1.15f },
//     { 1, { 4.1f, OutSideHeight, 11.0f, 1.0f }, 1.15f },
//     { 1, { -4.5f, OutSideHeight, 11.0f, 1.0f }, 1.15f },
//     { 1, { 0.0f, OutSideHeight, -11.0f, 1.0f }, 1.15f },
//     { 1, { 4.1f, OutSideHeight, -11.0f, 1.0f }, 1.15f },
//     { 1, { -4.5f, OutSideHeight, -11.0f, 1.0f }, 1.15f },

//     { 1, { 2.05f, OutSideHeight, 11.0f, 0.0f }, 1.15f },
//     { 1, { -2.25f, OutSideHeight, 11.0f, 0.0f }, 1.15f },
//     { 1, { 6.15f, OutSideHeight, 11.0f, 1.0f }, 1.15f },
//     { 1, { -6.75f, OutSideHeight, 11.0f, 1.0f }, 1.15f },

//     { 1, { 2.05f, OutSideHeight, -11.0f, 0.0f }, 1.15f },
//     { 1, { -2.25f, OutSideHeight, -11.0f, 0.0f }, 1.15f },
//     { 1, { 6.15f, OutSideHeight, -11.0f, 1.0f }, 1.15f },
//     { 1, { -6.75f, OutSideHeight, -11.0f, 1.0f }, 1.15f },

// #define TopHeight 8.0f
//     { 1, { 0.0f, TopHeight, 0.0f, 1.0f }, 1.15f },
//     { 1, { 4.1f, TopHeight, 0.0f, 0.0f }, 1.15f },
//     { 1, { -4.5f, TopHeight, 0.0f, 0.0f }, 1.15f },
//     { 1, { 8.2f, TopHeight, 0.0f, 1.0f }, 1.15f },
//     { 1, { -9.0f, TopHeight, 0.0f, 1.0f }, 1.15f },

//     { 1, { 2.05f, TopHeight, 0.0f, 0.0f }, 1.15f },
//     { 1, { -2.25f, TopHeight, 0.0f, 0.0f }, 1.15f },
//     { 1, { 6.15f, TopHeight, 0.0f, 1.0f }, 1.15f },
//     { 1, { -6.75f, TopHeight, 0.0f, 1.0f }, 1.15f },
};
#endif