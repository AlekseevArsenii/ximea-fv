#include<stdio.h>
#include "fastvideo_sdk.h"
#include<m3api/xiApi.h>
#include<memory.h>
#include<cstring>
#include<typeinfo>
#include<iostream>
#include<stdlib.h>
#include<cstdlib>
#include"helper_jpeg.hpp"
#include"helper_jpeg_store.cpp"


#define HandleResult(res,place) if (res!=XI_OK) {printf("Error after %s (%d)\n",place,res);goto finish;}
int main(int argc, char** argv)
{
	XI_IMG image;
	unsigned pitch = 2064;
	unsigned char * source;
	size_t allocatedsize = 0;
	memset(&image,0,sizeof(image));
	image.size = sizeof(XI_IMG);
	XI_RETURN stat = XI_OK;
	HANDLE xiH = NULL;
	size_t size = 0;
	fastStatus_t st = FAST_OK;
	fastImportFromHostHandle_t handle;
	fastSurfaceFormat_t surfaceFmt = FAST_I8;
	unsigned maxWidth = 2064;
	unsigned maxHeight = 1544;
	unsigned images = 0;
	fastDeviceSurfaceBufferHandle_t dstBuffer;
        unsigned affinity = 1;
	unsigned quality = 90;
	unsigned bufsize = maxWidth * maxHeight;
	fastJpegEncoderHandle_t jhandle;
	fastJfifInfo_t jfif;
	memset(&jfif, 0, sizeof(jfif));
	jfif.exifSectionsCount = 0;
	jfif.exifSections = NULL;
	jfif.jpegFmt = FAST_JPEG_Y;
	jfif.restartInterval = 32;
	jfif.bitsPerChannel = 8;
	fastMalloc((void**)&jfif.h_Bytestream, bufsize * sizeof(unsigned char));


        bool GL = false;
        st = fastInit(affinity, GL);
    st = fastImportFromHostCreate(&handle, surfaceFmt, maxWidth, maxHeight, &dstBuffer);
    printf("Import Create status : %d\n", st);
    st = fastImportFromHostGetAllocatedGpuMemorySize(handle, &allocatedsize);
    printf("Memory size : %ld Status : %d\n", allocatedsize, st);
    st = fastJpegEncoderCreate(&jhandle, maxWidth, maxHeight, dstBuffer);
    printf("Jpeg create status : %d\n", st);
    printf("Opening first camera...\n");
    stat = xiOpenDevice(0, &xiH);
    HandleResult(stat,"xiOpenDevice");


    // Setting "exposure" parameter (10ms=10000us)
    stat = xiSetParamInt(xiH, XI_PRM_EXPOSURE, 10000);
    HandleResult(stat,"xiSetParam (exposure set)");


    // Note:
    // The default parameters of each camera might be different in different API versions
    // In order to ensure that your application will have camera in expected state,
    // please set all parameters expected by your application to required value.

    printf("Starting acquisition...\n");
    stat = xiStartAcquisition(xiH);
    HandleResult(stat,"xiStartAcquisition");

    while(true)
    {
        stat = xiGetImage(xiH, 3000, &image);
        HandleResult(stat,"xiGetImage");
        unsigned char pixel = *(unsigned char*)image.bp;
        printf("Image %d (%dx%d) received from camera. First pixel value: %d padding: %d buffer size: %d\n", images, (int)image.width, (int)image.height, pixel, image.padding_x, image.bp_size);
	st = fastImportFromHostCopy(handle, image.bp, image.width, image.width, image.height); 
	printf("Copy status: %d\n", st);
	st = fastJpegEncode(jhandle, quality, &jfif);
	printf("Encoding status : %d\n", st);
	char defaults[100] = "/home/nvidia/Pictures/";
	char defaulte[10] = ".jpg";
	char nbuffer[1000];
	char filename[1000];
	snprintf(nbuffer, sizeof(nbuffer), "%d", images);
	strcpy(filename, defaults);
	strcat(filename, nbuffer);
	strcat(filename, defaulte);
	printf("filename = %s\n", filename);
	st = fastJfifStoreToFile(filename, &jfif);
	printf("Store status: %d\n", st);
	images++;
    }

    printf("Stopping acquisition...\n");
    xiStopAcquisition(xiH);
    xiCloseDevice(xiH);
    st = fastImportFromHostDestroy(handle);
    st = fastJpegEncoderDestroy(jhandle);
finish:
    printf("Done\n");
#ifdef WIN32
    Sleep(2000);
#endif
    return 0;
}

	//unsigned pitch = 4;
	//fastDeviceSurfaceBufferHandle_t dstBuffer;
	//unsigned affinity = 1;
	//bool GL = false;
	//printf("1: %d %d\n", st, stat);
	//printf("2: Image : %d %d %d\n", image.width, image.height, image.bp_size);
	//st = fastInit(affinity, GL);
	//printf("3: st = %d\n", st);
	//st = fastRawImportFromHostCreate(&handle, rawFmt, &surfaceFmt, maxWidth, maxHeight, &dstBuffer);
	//printf("4: st = %d\n", st);
	//size_t bufsize = maxWidth * maxHeight;
	//unsigned char * src = malloc(bufsize);
	//std::cout << typeid().name() <<std::endl;
	//memcpy(src, image.bp, bufsize);
	//unsigned char * pixel = (unsigned char*)image.bp;
	//printf("5: pixel = %c, size = %d\n", *(pixel + 100), image.bp_size);
//	st = fastRawImportFromHostGetAllocatedGpuMemorySize(handle, &size);
	//st = fastRawImportFromHostDecode(handle, image.bp, pitch, maxWidth, maxHeight);
//	printf("6: %d %d %ld\n", st, stat, size);
	//st = fastRawImportFromHostDestroy(handle);

