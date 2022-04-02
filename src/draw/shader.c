#include "draw/shader.h"

static void shaderStatus(shaderType shader, int32_t type) {
    printf("Compilling...\n");
    int32_t status;
    if (type == PROGRAM) {
        glGetShaderiv(shader, GL_LINK_STATUS, &status);
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    }
    if (status == GL_FALSE) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        printf("[SHADER_%s_ERROR]", type == PROGRAM ? "LINKING" : "COMPILE");
        switch (type) {
            case PROGRAM:   printf(" for: PROGRAM\n");       break;
            case VERTEX:    printf(" for: VERTEX\n");        break;
            case FRAGMENT:  printf(" for: FRAGMENT\n");      break;
            default: printf(" for: UNKNOW SHADER TYPE\n");  break;
        }
        printf("%s\n", infoLog);
    } else {
        printf("Succesful\n\n");
    }
}

static const char* shaderContent(const char* filename) {
    printf("OPENING SHADER: %s\n", filename);
    FILE* file = fopen(filename, "r");
    ASSERT(file == NULL, SHADER NOT OPENED)
    
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char* content = calloc(size, sizeof(char));
    size_t read = fread(content, sizeof(char), size, file);
    fclose(file);

    WARNING(read != size, SHADER SOURCE READ FAILED)

    return content;
}

void initShader(Shader* shader, const char* vertPath, const char* fragPath) {
    // VERTEX SHADER
    snprintf(shader->vertFile, MAX_SHADER_PATH_LENGHT, "%s", vertPath);
    const char* vertexSource = shaderContent(shader->vertFile);

    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    shaderStatus(vertexShader, VERTEX);
    free((void*)vertexSource);


    // FRAGMENT SHADER
    snprintf(shader->fragFile, MAX_SHADER_PATH_LENGHT, "%s", fragPath);        
    const char* fragmentSource = shaderContent(shader->fragFile);

    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    shaderStatus(fragmentShader, FRAGMENT);
    free((void*)fragmentSource);

    // LINKING PROGRAM TOGETHER
    shader->shaderID = glCreateProgram();

    glAttachShader(shader->shaderID, vertexShader);
    glAttachShader(shader->shaderID, fragmentShader);

    glLinkProgram(shader->shaderID);
    glValidateProgram(shader->shaderID);
    printf("ATTACHING SHADERS\n");
    shaderStatus(shader->shaderID, PROGRAM);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}