#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8081
#define BUFFER_SIZE 1024

typedef struct
{
    long image_size;
    uint8_t *image_blob;
} CatImage;

CatImage *return_cat_photo()
{
    CatImage *cat_image = malloc(sizeof(CatImage));

    FILE *file = fopen("./cat.jpg", "rb");
    cat_image->image_size = 0;
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open the file\n");
        return cat_image;
    }
    fseek(file, 0, SEEK_END);
    cat_image->image_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    cat_image->image_blob = (uint8_t *)calloc(cat_image->image_size, sizeof(uint8_t));
    if (cat_image->image_blob == NULL)
    {
        fprintf(stderr, "Memory allocation for cat_image->image_blob failed\n");
        fclose(file);
        return cat_image;
    }

    fread(cat_image->image_blob, 1, cat_image->image_size, file);
    fclose(file);
    return cat_image;
}

int blank_space_pos(char *buffer)
{
    int i = 0;
    while (1)
    {
        if (buffer[i] == ' ')
        {
            break;
        }
        i++;
    }

    return i;
}

char *route_path(char *buffer, int start_point, int end_point)
{
    int char_length = end_point;
    char *path = malloc((char_length) * sizeof(char));
    for (int i = 0; i < char_length; i++)
    {
        path[i] = buffer[i + (start_point + 1)];
    }

    return path;
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        printf("\nWaiting for a connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed");
            continue;
        }

        read(new_socket, buffer, BUFFER_SIZE);
        int first_blank_pos = blank_space_pos(buffer);
        int last_blank_pos = blank_space_pos(buffer + (first_blank_pos + 1));
        char *path = route_path(buffer, first_blank_pos, last_blank_pos);

        printf("Received Request:\n%s\n", buffer);
        printf("Bufffer on 1st pos: %c\n", buffer[0]);
        printf("Blank pos: %i\n", first_blank_pos);
        printf("Blank pos last: %i\n", last_blank_pos);
        printf("PATH: %s\n", path);

        // char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from C!";
        CatImage *cat_resp = return_cat_photo();
        char *response = "HTTP/1.1 200 OK\r\nContent-Type: image/jpg\r\n\r\n";

        size_t header_len = strlen(response);
        size_t total_size = header_len + cat_resp->image_size;

        uint8_t *full_response = malloc(total_size);

        if (full_response != NULL)
        {
            memcpy(full_response, response, header_len);

            memcpy(full_response + header_len, cat_resp->image_blob, cat_resp->image_size);
        }

        write(new_socket, full_response, total_size);
        free(full_response);
        free(cat_resp->image_blob);
        free(cat_resp);
        close(new_socket);
    }

    return 0;
}