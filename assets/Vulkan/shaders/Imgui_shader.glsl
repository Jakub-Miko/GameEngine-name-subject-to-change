#RootSignature
{
	"RootSignature": [
		{
			"name" : "conf",
			"type" : "constant_buffer"
		},
        {
			"name" : "Texture",
			"type" : "texture_2D"
		}

	]
}
#end
)"        
#Vertex //--------------------------------------------------
#version 430

layout(set = 0, binding = 0) uniform conf 
{
   mat4 ProjMtx;
};
layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec4 Color;
out vec2 Frag_UV;
out vec4 Frag_Color;
void main()
{
    Frag_UV = UV;
    Frag_Color = Color;
    gl_Position = ProjMtx * vec4(Position.xy,0,1);
}
#end
#Fragment //------------------------------------------------
#version 430
in vec2 Frag_UV;
in vec4 Frag_Color;
layout(set = 0, binding = 1) uniform sampler2D Texture[1];
layout (location = 0) out vec4 Out_Color;
void main()
{
    Out_Color = Frag_Color * texture(Texture[0], Frag_UV.st);
}
#end