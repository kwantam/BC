#ifdef DEBUG
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#define _RSW_NUM_COEFFICIENTS 17
#define _RSW_NUM_BOOLS 3
#define _RSW_MAX_NAMELENGTH 256
#define _RSW_MAX_SERLENGTH 512

typedef struct {
        char name[_RSW_MAX_NAMELENGTH];
        double coefficients[_RSW_NUM_COEFFICIENTS];
} savedCalc;

void strrepl (char *str, char from, char to)
{
        int i=-1;
        while ( str[++i] != '\0' ) { if ( str[i] == from ) { str[i] = to; } }
}

// turn a savedCalc structure into a string to be saved to a file
// return number of bytes in serialized string, not including '\0'
int serializeCalc (savedCalc *calc, char *scalc)
{
        int i, size, bufstart=0;

        // we use ':' as a field separator, so replace it with a
        // non-typable character so that we can restore ':' contained
        // in the name when we retrieve the structure
        strrepl(calc->name,':','\1');
        
        // start by writing the name
        bufstart = snprintf(scalc,_RSW_MAX_NAMELENGTH,"%s",calc->name);

        // repeatedly overwrite the trailing \0 with the next string chunk
        for (i=0;i<_RSW_NUM_COEFFICIENTS;i++) {
                size = snprintf((char *) (scalc+bufstart*sizeof(char)),
                                _RSW_MAX_SERLENGTH-bufstart, ":%e",
                                (calc->coefficients)[i]);
                bufstart += size;
        }

        // add in the final '\n\0'
        size = snprintf((char *) (scalc+bufstart*sizeof(char)),
                        _RSW_MAX_SERLENGTH-bufstart, "\n");
        scalc[_RSW_MAX_SERLENGTH-1] = '\0';
        bufstart += size;

        return bufstart;
}

// serialize the calc data into a string to be written to a file
// mallocs the string and puts the pointer in **contents
// returns size of string
int serializeFile (savedCalc **calc, char **contents)
{
        char sBuf[_RSW_MAX_SERLENGTH];
        int i, size, bufstart = 0;
        char *sFile = NULL;

        for (i=0;calc[i]!=NULL;i++)
        {
                size = serializeCalc(calc[i], sBuf);
                sFile = (char *) realloc((void *) sFile, (bufstart+size+1)*sizeof(char));

                // overwrite the previously written null terminator with the new string start
                strncpy((char *) (sFile+bufstart*sizeof(char)), sBuf, size+1);

                // increase bufstart by size; do not include size of null terminator (per above)
                bufstart += size;
        }

        // make sure we allocate and return something, even if it's just an empty string
        sFile = (char *) realloc((void *) sFile, (bufstart+1)*sizeof(char));
        sFile[bufstart] = '\0';
        *contents = sFile;
        return bufstart;
}

// turn a string from a file into a savedCalc structure
// this function mallocs a new savedCalc struct which should
// be freed later, and returns a pointer to it
savedCalc *deserializeCalc (char *scalc)
{
        int i;
        savedCalc *calc = (savedCalc *) malloc(sizeof(savedCalc));
        char *scalcTok;
        char *tokptr = NULL;
        
        // read in the name
        // be sure that we have a '\0' trailing
        // revert '\1' to ':'
        scalcTok = strtok_r(scalc,":",&tokptr);
        strncpy(calc->name,scalcTok,_RSW_MAX_NAMELENGTH);
        (calc->name)[_RSW_MAX_NAMELENGTH-1] = '\0';
        strrepl(calc->name,'\1',':');

        // read in each coefficient, convert it to double
        for (i=0;i<_RSW_NUM_COEFFICIENTS;i++) {
                if ((scalcTok = strtok_r(NULL,":",&tokptr)) == NULL) {
                        (calc->coefficients)[i] = 0;
                }
                else { (calc->coefficients)[i] = strtod(scalcTok,NULL); }
        }

        return calc;
}

// take the text of a file and turn it into an array of
// savedCalc struct pointers
// returns the number of calcs serialized, and updates calc
// with the location of the new array of struct pointers
// this function mallocs the array, which should be freed later
int deserializeFile (char *savefile, savedCalc ***calc)
{
        int i = 0;
        savedCalc **calcs = NULL;
        char *savefileTok;
        char *tokptr = NULL;

        // if savefile is NULL, just initialize the calcs array for later use
        if (savefile != NULL) {
                savefileTok = strtok_r(savefile,"\n",&tokptr);
                while (savefileTok != NULL)
                {
                        calcs = (savedCalc **) realloc((void *) calcs, (i+1)*sizeof(savedCalc *));
                        calcs[i++] = deserializeCalc(savefileTok);
                        savefileTok = strtok_r(NULL,"\n",&tokptr);
                }
        }

        // add NULL termination to the end of the array
        calcs = (savedCalc **) realloc((void *) calcs, (i+1)*sizeof(savedCalc *));
        calcs[i] = NULL;

        *calc = calcs;
        return i;
}

// allocate a new structure from the components
savedCalc *newCalc (char *name, double *coefficients)
{
        int i;

        // copy in the name
        // make sure it doesn't overrun the allocated space in the struct
        savedCalc *nC = (savedCalc *) malloc(sizeof(savedCalc));
        strncpy(nC->name,name,_RSW_MAX_NAMELENGTH);
        nC->name[_RSW_MAX_NAMELENGTH-1] = '\0';

        // copy the coefficients
        for (i=0;i<_RSW_NUM_COEFFICIENTS;i++) { (nC->coefficients)[i] = coefficients[i]; }

        // return pointer to the new structure
        return nC;
}


// copy a calc struct from src to dest
savedCalc *calccpy (savedCalc *dest, const savedCalc *src)
{
        int i;

        // copy in the name
        // make sure it doesn't overrun the allocated space in the struct
        strncpy(dest->name, src->name, _RSW_MAX_NAMELENGTH);
        dest->name[_RSW_MAX_NAMELENGTH-1] = '\0';

        // copy the coefficients
        for (i=0;i<_RSW_NUM_COEFFICIENTS;i++) {
                (dest->coefficients)[i] = (src->coefficients)[i];
        }

        return dest;
}

// add a struct to the list of structs we have so far
// return number of structs in list
// "add" will malloc a structure and copy the contents
// of newCalc into the newly malloced memory
int addCalc (savedCalc *newCalc, savedCalc ***calcs)
{
        int i, done=0;

        for (i=0;(*calcs)[i]!=NULL;i++) {
                if ((*calcs)[i] == newCalc) { done = 1; }
                else if (strcmp((*calcs)[i]->name, newCalc->name)==0) {
                        // same name, copy new into old
                        calccpy((*calcs)[i],newCalc);
                        done = 1;
                }
        }

        // if we're done, return number of 
        if (done) { return i; }

        *calcs = (savedCalc **) realloc((void *) *calcs, (i+2)*sizeof(savedCalc *));
        (*calcs)[i] = (savedCalc *) malloc(sizeof(savedCalc));
        calccpy((*calcs)[i],newCalc);
        (*calcs)[i+1] = NULL;
        return (i+1);
}

// like addCalc, but insertCalc assumes that the argument
// has already been malloced.
// thus, if it finds another entry with a matching name,
// it will free() the old entry and just link in the
// new one
int insertCalc (savedCalc *newCalc, savedCalc ***calcs)
{
        int i, done=0;

        for (i=0;(*calcs)[i]!=NULL;i++) {
                // if they're identical pointers, don't free them!
                if ((*calcs)[i] == newCalc) { done = 1; }
                // same name, free old and point to new
                else if (strcmp((*calcs)[i]->name, newCalc->name)==0) {
                        free((*calcs)[i]);
                        (*calcs)[i] = newCalc;
                        done = 1;
                }
        }

        // if we're done, return number of structs
        if (done) { return i; }

        *calcs = (savedCalc **) realloc((void *) *calcs, (i+2)*sizeof(savedCalc *));
        (*calcs)[i] = newCalc;
        (*calcs)[i+1] = NULL;
        return (i+1);
}

// remove the nth element of the struct list
// return new length
int delCalc (int n, savedCalc ***calcs)
{
        int i;
        int done = 0;

	// don't try to free somewhere outside the legal bounds
	if (n < 0) { return -1; }

        // start at the beginning of the list, and double
        // check that the element we want to remove is indeed
        // in the list
        for (i=0;i<=n;i++) {
                if ((*calcs)[i] == NULL) { done = 1; break; }
        }
        
        if (!done) {
                // free the nth element
                // shift all elements after the nth down by one
                for (free((*calcs)[n]);(*calcs)[n]!=NULL;n++) { (*calcs)[n] = (*calcs)[n+1]; }

                // reallocate the array, reducing its size by one
                *calcs = (savedCalc **) realloc((void *) (*calcs), n*sizeof(savedCalc *));

                return (n-1);
        } else {
                return i;
        }
}

// number of entries in the calc array
int lengthCalc (savedCalc **calc)
{
        int i=0;

        while (calc[i] != NULL) { i++; }

        return i;
}

// free all elements of the struct list, and the list itself
void freeCalc (savedCalc **calc)
{
        int i;

        // free all the allocated structs
        for (i=0;calc[i]!=NULL;i++) { free(calc[i]); }

        // free the array itself
        free(calc);

        return;
}

#ifdef DEBUG
int main (void)
{
//        g_type_init();
//        GFile *savefile = g_file_new_for_path("~/.bcrc");
//        char *savecontents = NULL;
//        gsize savecontentlength;

        int sfoo;
        savedCalc **bar;
        savedCalc *quux;

        char *ser = "a:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16\nb:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16\nc:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16\nd:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16\ne:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16\nf:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16\n";
        char *serd = strdup(ser);

        savedCalc *sc1 = newCalc("g",(double[]){1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1});
        savedCalc *sc2 = newCalc("h",(double[]){1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1});
        savedCalc *sc3 = newCalc("i",(double[]){1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1});
        
        double d1=1;

        /*

        if (g_file_load_contents( savefile, NULL, &savecontents, &savecontentlength, NULL, NULL )) {
                printf("Success.\n--\n%s\n--\n",savecontents);
        } else {
                printf("Failure reading file.\n");
        }
        */

        sfoo = deserializeFile(serd, &bar);
        printf("%d deserialized\n", sfoo);

        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));
        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));
        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));

        printf("Adding sc2. Now %d.\n", addCalc(sc2, &bar));
        printf("Adding sc2. Now %d.\n", addCalc(sc2, &bar));
        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));

        printf("Adding sc3. Now %d.\n", addCalc(sc3, &bar));
        printf("Adding sc2. Now %d.\n", addCalc(sc2, &bar));
        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));

        printf("Adding bar[0]. Now %d.\n", addCalc(bar[0], &bar));

        free(serd);

        sfoo = serializeFile(bar, &serd);
        printf("%d bytes serialized\n", sfoo);
        printf("%s", serd);
        free(serd);

        printf("Deleting entry 5. Now %d.\n", delCalc(5, &bar));
        printf("Deleting entry 15. Now %d.\n", delCalc(15, &bar));

        sfoo = serializeFile(bar, &serd);
        printf("%d bytes serialized\n", sfoo);
        printf("%s", serd);

        freeCalc(bar);

        sfoo = deserializeFile(serd, &bar);
        printf("%d deserialized\n", sfoo);

        free(serd);

        sfoo = serializeFile(bar, &serd);
        printf("%d bytes serialized\n", sfoo);
        printf("%s", serd);
        free(serd);

        printf("%d entries\n", lengthCalc(bar));
        printf("%d entries\n", lengthCalc(bar));

        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));
        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));
        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));

        printf("Deleting entry 0. Now %d.\n", delCalc(0, &bar));
        printf("Deleting entry 0. Now %d.\n", delCalc(0, &bar));
        printf("Deleting entry 0. Now %d.\n", delCalc(0, &bar));
        printf("Deleting entry 0. Now %d.\n", delCalc(0, &bar));
        printf("Deleting entry 0. Now %d.\n", delCalc(0, &bar));
        printf("Deleting entry 0. Now %d.\n", delCalc(0, &bar));
        printf("Deleting entry 0. Now %d.\n", delCalc(0, &bar));

        printf("Adding sc2. Now %d.\n", addCalc(sc2, &bar));
        printf("Adding sc2. Now %d.\n", addCalc(sc2, &bar));
        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));

        printf("Adding sc3. Now %d.\n", addCalc(sc3, &bar));
        printf("Adding sc2. Now %d.\n", addCalc(sc2, &bar));
        printf("Adding sc1. Now %d.\n", addCalc(sc1, &bar));

        printf("Adding bar[0]. Now %d.\n", addCalc(bar[0], &bar));

        printf("%d entries\n", lengthCalc(bar));

        sfoo = serializeFile(bar, &serd);
        printf("%d bytes serialized\n", sfoo);
        printf("%s", serd);
        free(serd);

        quux = newCalc("asd:::f:", (double []){1,1,1,d1,d1,d1,d1,d1,d1,0.50231,d1,d1,d1,d1,d1,d1});
        printf("Adding quux. Now %d.\n", insertCalc(quux, &bar));
        printf("Adding quux. Now %d.\n", insertCalc(quux, &bar));

        sfoo = serializeFile(bar, &serd);
        printf("%d bytes serialized\n", sfoo);
        printf("%s", serd);
        free(serd);

        freeCalc(bar);
        free(sc1);
        free(sc2);
        free(sc3);

        return 0;
}
#endif
