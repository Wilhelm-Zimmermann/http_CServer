#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PORT 8081
#define BUFFER_SIZE 1024
#define PATHS_QTD 3

typedef struct
{
    long image_size;
    uint8_t *image_blob;
} Image;

typedef struct
{
    char *content_type;
    char *content;
    long response_size;
} Response;

Image *get_file(char *image_path)
{
    Image *file_info = malloc(sizeof(Image));

    FILE *file = fopen(image_path, "rb");
    file_info->image_size = 0;
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open the file\n");
        return file_info;
    }
    fseek(file, 0, SEEK_END);
    file_info->image_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    file_info->image_blob = (uint8_t *)calloc(file_info->image_size, sizeof(uint8_t));
    if (file_info->image_blob == NULL)
    {
        fprintf(stderr, "Memory allocation for file_info->image_blob failed\n");
        fclose(file);
        return file_info;
    }

    fread(file_info->image_blob, 1, file_info->image_size, file);
    fclose(file);
    return file_info;
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
    char *path = malloc(((char_length) * sizeof(char)) + 1);
    for (int i = 0; i < char_length; i++)
    {
        path[i] = buffer[i + (start_point + 1)];
    }
    path[char_length] = '\0';
    return path;
}

Response route_redirect(char *requested_path)
{
    Response resp;
    printf("Request PATH: %s\n", requested_path);
    if (strcmp(requested_path, "/favicon.ico") == 0)
    {
        Image *cat_photo = get_file("./cat.jpg");
        resp.content = cat_photo->image_blob;
        resp.content_type = "Content-Type: image/jpg\r\n\r\n";
        resp.response_size = cat_photo->image_size;
    }
    else if (strcmp(requested_path, "/cat") == 0)
    {
        Image *cat_photo = get_file("./cat.jpg");
        resp.content = cat_photo->image_blob;
        resp.content_type = "Content-Type: image/jpg\r\n\r\n";
        resp.response_size = cat_photo->image_size;
    }
    else if (strcmp(requested_path, "/dog") == 0)
    {
        Image *dog_photo = get_file("./dog.jpg");
        resp.content = dog_photo->image_blob;
        resp.content_type = "Content-Type: image/jpg\r\n\r\n";
        resp.response_size = dog_photo->image_size;
    }
    else if (strcmp(requested_path, "/") == 0)
    {
        Image *html_file = get_file("./index.html");
        resp.content = html_file->image_blob;
        resp.content_type = "Content-Type: text/html\r\n\r\n";
        resp.response_size = html_file->image_size;
    }
    else
    {
        Image *html_file = get_file("./not_found.html");
        resp.content = html_file->image_blob;
        resp.content_type = "Content-Type: text/html\r\n\r\n";
        resp.response_size = html_file->image_size;
    }

    return resp;
}

int main()
{
    int server_fd, new_socket;
    int opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
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
        Response resp = route_redirect(path);
        free(path);
        char *response = "HTTP/1.1 200 OK\r\n";

        size_t header_len = strlen(response);
        size_t resp_content_type_size = strlen(resp.content_type);
        size_t total_size = header_len + resp.response_size + resp_content_type_size;

        uint8_t *full_response = malloc(total_size);

        if (full_response != NULL)
        {
            memcpy(full_response, response, header_len);
            memcpy(full_response + header_len, resp.content_type, resp_content_type_size);
            memcpy(full_response + header_len + resp_content_type_size, resp.content, resp.response_size);
        }

        write(new_socket, full_response, total_size);
        free(full_response);
        close(new_socket);
    }

    return 0;
}