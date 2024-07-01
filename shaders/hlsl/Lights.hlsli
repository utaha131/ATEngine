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
        if (distance <= light.Radius) {
            light_sample.L = float3(1.0f, 1.0f, 1.0f) / (distance * distance);
        } else {
            light_sample.L = float3(0.0f, 0.0f, 0.0f);
        }
        light_sample.pdf = 1.0f;
        light_sample.wi_distance = min(light.Radius, distance);
    }
    return light_sample;
}

//Hardcoded Scene Lights for now.

#define LIGHT_COUNT 43

#define FirstHeight 0.5f

static Light Lights[LIGHT_COUNT] = {
    // //Directional
    // { 0, { 0.0f, 1.0f, 0.2f, 0.0f }, 0.0f },

    //1st Level.
    { 1, { 0.0f, FirstHeight, 0.0f, 1.0f }, 1.15f },
    { 1, { 4.1f, FirstHeight, 0.0f, 0.0f }, 1.15f },
    { 1, { -4.5f, FirstHeight, 0.0f, 0.0f }, 1.15f },
    { 1, { 8.2f, FirstHeight, 0.0f, 1.0f }, 1.15f },
    { 1, { -9.0f, FirstHeight, 0.0f, 1.0f }, 1.15f },

    { 1, { 2.05f, FirstHeight, 0.0f, 0.0f }, 1.15f },
    { 1, { -2.25f, FirstHeight, 0.0f, 0.0f }, 1.15f },
    { 1, { 6.15f, FirstHeight, 0.0f, 1.0f }, 1.15f },
    { 1, { -6.75f, FirstHeight, 0.0f, 1.0f }, 1.15f },


    { 1, { 0.0f, FirstHeight, 3.5f, 1.0f }, 1.15f },
    { 1, { 4.1f, FirstHeight, 3.5f, 0.0f }, 1.15f },
    { 1, { -4.5f, FirstHeight, 3.5f, 0.0f }, 1.15f },
    { 1, { 8.2f, FirstHeight, 3.5f, 1.0f }, 1.15f },
    { 1, { -9.0f, FirstHeight, 3.5f, 1.0f }, 1.15f },

    { 1, { 2.05f, FirstHeight, 3.5f, 0.0f }, 1.15f },
    { 1, { -2.25f, FirstHeight, 3.5f, 0.0f }, 1.15f },
    { 1, { 6.15f, FirstHeight, 3.5f, 1.0f }, 1.15f },
    { 1, { -6.75f, FirstHeight, 3.5f, 1.0f }, 1.15f },

    { 1, { 0.0f, FirstHeight, -3.5f, 1.0f }, 1.15f },
    { 1, { 4.1f, FirstHeight, -3.5f, 0.0f }, 1.15f },
    { 1, { -4.5f, FirstHeight, -3.5f, 0.0f }, 1.15f },
    { 1, { 8.2f, FirstHeight, -3.5f, 1.0f }, 1.15f },
    { 1, { -9.0f, FirstHeight, -3.5f, 1.0f }, 1.15f },

    { 1, { 2.05f, FirstHeight, -3.5f, 0.0f }, 1.15f },
    { 1, { -2.25f, FirstHeight, -3.5f, 0.0f }, 1.15f },
    { 1, { 6.15f, FirstHeight, -3.5f, 1.0f }, 1.15f },
    { 1, { -6.75f, FirstHeight, -3.5f, 1.0f }, 1.15f },


    //2nd Level.
    { 1, { 8.2f, 4.0f, 0.0f, 1.0f }, 1.15f },
    { 1, { -9.0f, 4.0f, 0.0f, 1.0f }, 1.15f },

    { 1, { 0.0f, 4.0f, 3.5f, 1.0f }, 1.15f },
    { 1, { 4.1f, 4.0f, 3.5f, 1.0f }, 1.15f },
    { 1, { -4.5f, 4.0f, 3.5f, 1.0f }, 1.15f },
    { 1, { 0.0f, 4.0f, -3.5f, 1.0f }, 1.15f },
    { 1, { 4.1f, 4.0f, -3.5f, 1.0f }, 1.15f },
    { 1, { -4.5f, 4.0f, -3.5f, 1.0f }, 1.15f },

    { 1, { 2.05f, 4.0f, 3.5f, 0.0f }, 1.15f },
    { 1, { -2.25f, 4.0f, 3.5f, 0.0f }, 1.15f },
    { 1, { 6.15f, 4.0f, 3.5f, 1.0f }, 1.15f },
    { 1, { -6.75f, 4.0f, 3.5f, 1.0f }, 1.15f },

    { 1, { 2.05f, 4.0f, -3.5f, 0.0f }, 1.15f },
    { 1, { -2.25f, 4.0f, -3.5f, 0.0f }, 1.15f },
    { 1, { 6.15f, 4.0f, -3.5f, 1.0f }, 1.15f },
    { 1, { -6.75f, 4.0f, -3.5f, 1.0f }, 1.15f },
};