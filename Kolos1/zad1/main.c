
/** Program identyfikuje typy plikow podane jako argumenty, rozpoznaje zwykle pliki,
    katalogi oraz linki symboliczne
    Nalezy uzupeni program w oznaczonych wierszach rozpoznajac odpowiednie 
    rodzeje plikow*/
#include <stdio.h>		
#include <stdlib.h>		
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    int i;
    struct stat buf;
    const char *tekst;

    /* zaczynamy od argv[1] – argv[0] to nazwa programu */
    for (i = 1; i < argc; i++) {
        printf("%s: ", argv[i]);

        if (lstat(argv[i], &buf) < 0) {
			printf("lstat error");
			continue;
		}

        if (S_ISREG(buf.st_mode))              /* zwykły plik          */
            tekst = "zwykly plik";
        else if (S_ISDIR(buf.st_mode))         /* katalog              */
            tekst = "katalog";
        else if (S_ISLNK(buf.st_mode)) {       /* link symboliczny     */
            tekst = "link symboliczny";
        }
        else
            tekst = "**** cos innego !!! ****";

        puts(tekst);
    }
    return 0;
}
