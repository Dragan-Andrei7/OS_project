#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
//primul argument din linia de comanda e un fiser text, iar al 2-lea argument reprezinta un fisier de iesire care se creeaza in interiorul programului(text)
//programul va parcurge fisierul de intrare si realizeaza statistica de numarare al caracterelor alfanumerice(litere, cifre)


int main (int argc, char *argv[])
{
    if(argc!=3)
    {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(1);
    }
    int inputFile = open(argv[1], O_RDONLY);
    if (inputFile == -1)
    {
        perror("Error opening input file");
        exit(1);
    }
    //FILE *outputFile = open(argv[2], O_RDWR|O_CREAT ,0644);
    int outputFile= open(argv[2], O_RDWR|O_CREAT, 0644);
    if (outputFile == -1)
    {
        perror("Error opening output file");
        fclose(inputFile);
        exit(1);
    }


    int c;
    char buffer[2048];
    unsigned int count = 0;
    int flag;
    while ((flag = read(inputFile, buffer, 2048))!= 0)
    {
        if (flag == -1)
        {
            perror("Error reading input file");
            close(inputFile);
            close(outputFile);
            exit(1);
        }
        for (int i = 0; i < flag; i++)
        {
            if (isalnum(buffer[i]))
            {
                count++;
            }
        }
    }

    printf("Number of alphanumeric characters: %u\n", count);
    write(outputFile, &count, sizeof(count));

    close(inputFile);
    close(outputFile);
}