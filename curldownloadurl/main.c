#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

typedef struct ImageBytes {
    unsigned char *image_data;
    size_t size;
} ImageBytes;

static size_t readImageByteChunk(char *data, size_t size, size_t nmemb, void *clientp)
{
    size_t total = nmemb * size;
    ImageBytes *mem = (ImageBytes *)clientp;

    unsigned char *ptr = realloc(mem->image_data, mem->size + total);
    if(!ptr)
        return 0;  /* out of memory */

    mem->image_data= ptr;
    memcpy(&(mem->image_data[mem->size]), data, total);
    // or
    // memcpy(mem->response + mem->size, data, realsize);
    mem->size += total;

    return total;
}


void writeTofile(const char *filename, const ImageBytes *chunk) {
    FILE *fp;

    size_t size = chunk->size;

    fp = fopen(filename, "wb");
    if(fp == NULL)
    {
        fprintf(stderr,"Error writing to %s\n",filename);
        return;
    }

    fwrite(chunk->image_data, size, 1, fp);

}

int main(void) {
    ImageBytes chunk = { 0 };
    CURLcode result;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "e621curl/1.0 (by Moneky on e621)");

        curl_easy_setopt(curl, CURLOPT_URL, "https://static1.e621.net/data/sample/df/cb/dfcb38b6c0cf45d5ad543ce96c5d8bc5.jpg");
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


        const char filename[] = "image.png";

        writeTofile(filename, &chunk);

        /* remember to free the buffer */
        free(chunk.image_data);
        curl_global_cleanup();
        return 0;
    }
}