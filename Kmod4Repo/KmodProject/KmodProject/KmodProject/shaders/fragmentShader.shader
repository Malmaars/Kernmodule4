#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 uv;
in mat3 tbn;
in vec3 worldPosition;

uniform sampler2D mainTex;
uniform sampler2D normalTex;

struct Light{
	vec3 position;
	vec3 color;
	};

uniform vec3 cameraPosition;

uniform Light lights[3];

void main()
{	//Normal map
	vec3 normal = texture(normalTex, uv).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	//transform with tbn
	normal = tbn * normal;

	//specular data
	vec3 viewDir = normalize(worldPosition - cameraPosition);

    // Texture color
    vec4 texColor = texture(mainTex, uv);
    vec3 fragColor = texColor.rgb * color;

    vec3 totalLight = vec3(0.0);

    for(int i = 0; i < 3; i++)
    {
        // Correct light direction calculation
        vec3 lightDir = normalize(lights[i].position - worldPosition);
        vec3 reflDir = reflect(-lightDir, normal);

        // Diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);

        // Specular shading
        float spec = 0.0;
        if (diff > 0.0) {
            spec = pow(max(dot(viewDir, reflDir), 0.0), 8);
        }

        // Accumulate light contributions
        vec3 diffuse = diff * lights[i].color;
        vec3 specular = spec * lights[i].color;

        totalLight += diffuse + specular;
    }

    fragColor = fragColor * totalLight * 2;

    FragColor = vec4(fragColor, texColor.a);

}

