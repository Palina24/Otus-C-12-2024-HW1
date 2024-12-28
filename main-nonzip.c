#include <stdio.h>
#include <stdlib.h>

struct LocalFileHeader
{
    // Обязательная сигнатура, равна 0x04034b50
    int32_t signature;
    int16_t versionToExtract;
    int16_t generalPurposeBitFlag;
    int16_t compressionMethod;
    int16_t modificationTime;
    int16_t modificationDate;
    int32_t crc32;
    int32_t compressedSize;
    int32_t uncompressedSize;
    // Длина название файла
    int16_t filenameLength;
    int16_t extraFieldLength;
   // Имя файла (длиной filenameLength)
    int8_t *filename;
    // Дополнительные данные (длиной extraFieldLength)
    int8_t *extraField;
};

short isLocalFileHeader(FILE* fp, struct LocalFileHeader* loc, short firstOutput){
    FILE* rp = fp;
    fread(&(loc->signature), sizeof(loc->signature), 1, rp);
    if(loc->signature == 0x04034b50){
            //значит, мы прочитали заголовок файла
            //игнорируем поля, которые нам неинтересны
            size_t toIgnore = sizeof(loc->versionToExtract)
                    + sizeof(loc->generalPurposeBitFlag)
                    + sizeof(loc->compressionMethod)
                    + sizeof(loc->modificationTime)
                    + sizeof(loc->modificationDate)
                    + sizeof(loc->crc32)
                    + sizeof(loc->compressedSize)
                    + sizeof(loc->uncompressedSize);
            fseek(rp, toIgnore, SEEK_CUR);

            //читаем длину имя файла
            fread(&(loc->filenameLength), sizeof(loc->filenameLength), 1, rp);

            //игнорируем длину комментария
            fseek(rp, sizeof(loc->extraFieldLength), SEEK_CUR);

            //выделяем память, записываем и распечатываем имя файла
            loc->filename = malloc(sizeof(int8_t)*(loc->filenameLength + 1));
            fread(loc->filename, sizeof(int8_t), loc->filenameLength, rp);
            loc->filename[loc->filenameLength] = 0;

            //если это - первая запись, пишем, что перед нами архив
            if(firstOutput) printf("Is archive. Contains files:\n");
            printf("filename: %s\n", loc->filename);
            free(loc->filename);
            return 1;
        } else if (loc->signature == 0x06054b50){
            //значит, мы дошди до конца архива
            //  printf("End of archive\n");
            return 2;
        } else return 0;
}
int main()
{
    printf("Print name of file to check, if it has zip-archive\n");
    printf("Print Ctrl+C to exit\n");
    while(1){
        unsigned fileCounter = 0;
        short isZip = 0;
        int buf;
        char filename[256];

        printf("Input filename: ");
        scanf("%s", filename);
        FILE* my_file = fopen(filename, "rb");

        if(my_file == NULL){
                perror("Error occured while opening the file\n");
                continue;
        }


        for(unsigned long i = 0; ; i++){
            buf = getc(my_file);
            if(buf == EOF)
                //дошли до конца файла-> прекращаем читать
                break;
            else if (buf != 0x50)
                //значит, отсюда точно не начинается запись в архиве
                continue;

            struct LocalFileHeader loc;
            fseek(my_file, -1, SEEK_CUR);
            short flag = isLocalFileHeader(my_file, &loc, !fileCounter);
            if(flag == 1)
                fileCounter ++ ;
            else if(flag == 2)
                isZip = 1;
            fseek(my_file, 1, SEEK_CUR);
        }
        if (!fileCounter && isZip)
            printf("Empty archive\n");
        else if (!fileCounter)
            printf("Not archive\n");

        fclose(my_file);
    }
   return 0;
}
