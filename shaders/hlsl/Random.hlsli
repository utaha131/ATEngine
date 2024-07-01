//Code From https://github.com/NVIDIAGameWorks/rtxdi-runtime/blob/5ee2a55fedf865b0a19409ae8d2b6539b937d797/include/rtxdi/RtxdiMath.hlsli#L69
//And https://github.com/NVIDIAGameWorks/RTXDI/blob/4a21ce6593e830987f2c1d5db179b3df18cf0225/minimal/shaders/HelperFunctions.hlsli#L23

struct RNGSampler {
    uint Seed;
    uint Index;

    uint murmur3() {
    #define ROT32(x, y) ((x << y) | (x >> (32 - y)))

        // https://en.wikipedia.org/wiki/MurmurHash
        uint c1 = 0xcc9e2d51;
        uint c2 = 0x1b873593;
        uint r1 = 15;
        uint r2 = 13;
        uint m = 5;
        uint n = 0xe6546b64;

        uint hash = Seed;
        uint k = Index++;
        k *= c1;
        k = ROT32(k, r1);
        k *= c2;

        hash ^= k;
        hash = ROT32(hash, r2) * m + n;

        hash ^= 4;
        hash ^= (hash >> 16);
        hash *= 0x85ebca6b;
        hash ^= (hash >> 13);
        hash *= 0xc2b2ae35;
        hash ^= (hash >> 16);

    #undef ROT32

        return hash;
    }

    float SampleUniform() {
        uint value = murmur3();
        const uint one = asuint(1.0f);
        const uint mask = (1 << 23) - 1;
        return asfloat((mask & value) | one) - 1.0f;
    }
};

uint IntegerExplode(uint x) {
    x = (x | (x << 8)) & 0x00FF00FF;
    x = (x | (x << 4)) & 0x0F0F0F0F;
    x = (x | (x << 2)) & 0x33333333;
    x = (x | (x << 1)) & 0x55555555;
    return x;
}

uint ZCurveToLinearIndex(uint2 xy)
{
    return IntegerExplode(xy[0]) | (IntegerExplode(xy[1]) << 1);
}

uint JenkinsHash(uint a)
{
    // http://burtleburtle.net/bob/hash/integer.html
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

RNGSampler InitRNGSampler(uint2 pixel_index, uint frame_number) {
    RNGSampler rng_sampler;
    uint linear_pixel_index = ZCurveToLinearIndex(pixel_index);
    rng_sampler.Index = 1;
    rng_sampler.Seed = JenkinsHash(linear_pixel_index) + frame_number;
    return rng_sampler;
}