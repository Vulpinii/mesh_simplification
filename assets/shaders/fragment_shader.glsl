#version 330 core

// Interpolated values from the vertex shaders
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
in float valence;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform mat4 MV;
uniform vec3 LightPosition_worldspace;
uniform bool show_valence;
uniform bool no_lighting;

// return float value normalized betwenn 0 and 1
vec3 HSV2RGB( in float value )
{

    vec3 hsv = vec3 (value, 1., 1.);
    hsv.x = mod( 100.0 + hsv.x, 1.0 ); // Ensure [0,1[
    float   HueSlice = 6.0 * hsv.x; // In [0,6[
    float   HueSliceInteger = floor( HueSlice );
    float   HueSliceInterpolant = HueSlice - HueSliceInteger; // In [0,1[ for each hue slice
    vec3  TempRGB = vec3(   hsv.z * (1.0 - hsv.y), hsv.z * (1.0 - hsv.y * HueSliceInterpolant), hsv.z * (1.0 - hsv.y * (1.0 - HueSliceInterpolant)) );
    float   IsOddSlice = mod( HueSliceInteger, 2.0 ); // 0 if even (slices 0, 2, 4), 1 if odd (slices 1, 3, 5)
    float   ThreeSliceSelector = 0.5 * (HueSliceInteger - IsOddSlice); // (0, 1, 2) corresponding to slices (0, 2, 4) and (1, 3, 5)
    vec3  ScrollingRGBForEvenSlices = vec3( hsv.z, TempRGB.zx );           // (V, Temp Blue, Temp Red) for even slices (0, 2, 4)
    vec3  ScrollingRGBForOddSlices = vec3( TempRGB.y, hsv.z, TempRGB.x );  // (Temp Green, V, Temp Red) for odd slices (1, 3, 5)
    vec3  ScrollingRGB = mix( ScrollingRGBForEvenSlices, ScrollingRGBForOddSlices, IsOddSlice );
    float   IsNotFirstSlice = clamp( ThreeSliceSelector, 0.0,1.0 );                   // 1 if NOT the first slice (true for slices 1 and 2)
    float   IsNotSecondSlice = clamp( ThreeSliceSelector-1.0, 0.0,1. );              // 1 if NOT the first or second slice (true only for slice 2)
    return  mix( ScrollingRGB.xyz, mix( ScrollingRGB.zxy, ScrollingRGB.yzx, IsNotSecondSlice ), IsNotFirstSlice );    // Make the RGB rotate right depending on final slice index
}

void main(){
    vec3 LightColor = vec3(1,1,1);
	float LightPower = 50.0f;
	
    vec3 MaterialDiffuseColor = vec3(0.71f, 0.22f, 0.27f);
    if(show_valence) MaterialDiffuseColor = HSV2RGB( valence );

	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

	float distance = length( LightPosition_worldspace - Position_worldspace );
	vec3 n = normalize( Normal_cameraspace );
	vec3 l = normalize( LightDirection_cameraspace );
	float cosTheta = clamp( dot( n,l ), 0,1 );
	vec3 E = normalize(EyeDirection_cameraspace);
	vec3 R = reflect(-l,n);
	float cosAlpha = clamp( dot( E,R ), 0,1 );

	if(no_lighting) color = MaterialDiffuseColor;
    else color = MaterialAmbientColor + MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) + MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
}
