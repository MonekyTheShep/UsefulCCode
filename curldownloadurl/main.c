#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

typedef struct ImageBytes {
    unsigned char *image_data;
    size_t size;
} ImageBytes;


typedef struct ReadFileResult {
    unsigned char *file_data;
    long size;
} ReadFileResult;

static size_t readImageByteChunk(unsigned char *data, size_t size, size_t nmemb, void *clientp)
{
    size_t total = nmemb * size;
    ImageBytes *mem = (ImageBytes *)clientp;

    unsigned char *ptr = realloc(mem->image_data, mem->size + total);
    // null terminate for strings
    //unsigned char *ptr = realloc(mem->image_data, mem->size + total + 1);
    if(!ptr)
        return 0;  /* out of memory */

    mem->image_data= ptr;
    memcpy(&(mem->image_data[mem->size]), data, total);
    // or
    // memcpy(mem->image_data + mem->size, data, total);
    mem->size += total;

    // Null terminate at start of next chunk
    // mem->image_data[mem->size] = 0;
    return total;
}


void writeToFile(const char *filename, const ImageBytes *chunk) {
    FILE *fp;

    fp = fopen(filename, "wb");
    if(fp == NULL)
    {
        fprintf(stderr,"Error writing to %s\n",filename);
        return;
    }

    fwrite(chunk->image_data, sizeof(unsigned char), chunk->size, fp);

}

void writeToFileChar(const char *filename, const unsigned char *chunk, const long size) {
    FILE *fp;

    fp = fopen(filename, "wb");
    if(fp == NULL)
    {
        fprintf(stderr,"Error writing to %s\n",filename);
        return;
    }

    fwrite(chunk, sizeof(unsigned char), size, fp);

}

unsigned char *readFromFileChar(const char *filename) {
    // Open the file for reading
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END); // seek to end of file
    long size = ftell(file); // get current file pointer
    rewind(file);

    unsigned char *buffer = malloc(size * sizeof(unsigned char) + 1);

    if (buffer == NULL) {
        fprintf(stderr, "Error allocating memory for buffer");
        fclose(file);
        return NULL;
    }


    // Read the integers from the file into the buffer
    fread(buffer, sizeof(unsigned char),size, file);
    fclose(file);
    buffer[size] = '\0';

    return buffer;
}

int main(void) {
    ImageBytes chunk = { 0 };
    CURLcode result;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if(curl) {
        const char url[] = "https://e621.net/posts.json?limit=2";

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "e621curl/1.0 (by Moneky on e621)");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readImageByteChunk);


        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* send a request */
        result = curl_easy_perform(curl);

        /* check for errors */
        if(result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result));

            curl_easy_cleanup(curl);
        }

        // write to file
        const char filename[] = "posts.json";
        writeToFile(filename, &chunk);



        // read and write back to file
        unsigned char *fileData = readFromFileChar(filename);

        size_t length = strlen((const char *)fileData);

        writeToFileChar(filename, fileData, (long) length);

        // Free the file data
        free(fileData);
        free(chunk.image_data);
        curl_global_cleanup();
        return 0;
    }

}
