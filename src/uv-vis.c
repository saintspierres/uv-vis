/*
 *  uv-vis.c
 *  
 *
 *  Created by Eric Bremond on 12/10/11.
 *  Copyright 2011 huckel.c. All rights reserved.
 *
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#include"uv-vis.h"

int main(int argc, char *argv[])
{
    int i, j;

    int ntrans = 0;
    int npts   = 0;
    
    double fwhm = .5;
    double trans[TAB_SIZE][CRD_SIZE]  = {{0.}};

    double xmin = 190.;
    double xmax = 800.;
    double xstp = 1.;

    double pts[650][CRD_SIZE] = {{0.}};

    char inpfile[LGN_SIZE] = "";
    char outfile[LGN_SIZE] = "";

    rcmdl(argc, argv, inpfile, outfile, &fwhm);

    rgauss(inpfile, &ntrans, trans, fwhm);
    if (ntrans == 0)
    {
        rtrans(inpfile, &ntrans, trans, fwhm);
    }

    printf(" %s: %d transitions\n", inpfile, ntrans);

    for (i=0; i<ntrans; i++)
    {
       printf("%12.6lf%12.6lf%12.6lf\n", trans[i][0], trans[i][1], trans[i][2]);
    }

    spectrum(&npts, pts, xmin, xmax, xstp, ntrans, trans);
/*
    for (i=0; i<npts; i++)
    {
       printf("%12.6lf%12.6lf\n", pts[i][0], pts[i][1]);
    }
*/
    wspec(outfile, npts, pts);

    return 0;
}

void usage()
{
	fprintf(stderr, "usage: uv-vis [options] [inputfile]\n\n");
        fprintf(stderr, "   -i           inpfile\n");
	fprintf(stderr, "   -o           outfile\n");
	fprintf(stderr, "   --fwhm val   set fwhm to val\n");

	exit(0);
}

void rcmdl(int argc, char *argv[], char *inpfile, char *outfile, double *fwhm)
{
    int i;

    if (argc==1)
    {
        usage();
    }
    else if (argc==2)
    {
       strcpy(inpfile, argv[1]);
    }

    for (i=1; i<argc; i++)
    {
        if (!strcmp(argv[i], "-h"))
        {
            usage();
        }
        else if (!strcmp(argv[i], "-i"))
        {
            if (i<argc-1)
            {
                strcpy(inpfile, argv[i+1]);
            }
            else
            {
                fprintf(stderr, "uv-vis: missing argument for -i\n");
            }
        }
        else if (!strcmp(argv[i], "-o"))
        {
            if (i<argc-1)
            {
                strcpy(outfile, argv[i+1]);
            }
            else
            {
                fprintf(stderr, "uv-vis: missing argument for -o\n");
            }
        }
        else if (!strcmp(argv[i], "--fwhm"))
        {
            if (i<argc-1)
            {
                *fwhm=atoi(argv[i+1]);
            }
            else
            {
                fprintf(stderr, "uv-vis: missing argument for --fwhm\n");
            }
        }
    }

    if (inpfile[0] == '\0' || argc>1)
    {
        strcpy(inpfile, argv[1]);
    }

    if (outfile[0] == '\0')
    {
        strcpy(outfile, "stdout");
    }
}

void rtrans(char *filename, int *ntrans, double trans[][CRD_SIZE], double fwhm)
{
    int i;
    char lign[LGN_SIZE] = "";

    FILE *pfile = NULL;
    pfile = fopen(filename, "r");

    if (pfile==NULL)
    {
        fprintf(stderr, "uv-vis: %s: No such file or directory\n", filename);
        usage();
    }

    fscanf(pfile, "%d", ntrans);
    fgets(lign, LGN_SIZE, pfile);
    fgets(lign, LGN_SIZE, pfile);

    for (i=0; i<*ntrans; i++)
    {
        fgets(lign, LGN_SIZE, pfile);
        sscanf(lign, "%lf%lf%lf",  &trans[i][0], &trans[i][1], &trans[i][2]);
        if (trans[i][2]==0.)
        {
          trans[i][2]=fwhm;
        }
    }

    fclose(pfile);
}

void rgauss(char *filename, int *ntrans, double trans[][CRD_SIZE], double fwhm)
{
    int i;
    char lign[LGN_SIZE] = "";
    char *pchr = NULL;

    FILE *pfile = NULL;
    pfile = fopen(filename, "r");

    if (pfile==NULL)
    {
        fprintf(stderr, "uv-vis: %s: No such file or directory\n", filename);
        usage();
    }

    while (fgets(lign, LGN_SIZE, pfile) != NULL)
    {
        if (strstr(lign, "Excited State") != NULL)
        {
            pchr = strtok(lign, " ");
            i = 0;
            while (pchr != NULL)
            {
                if (i==4)
                {
                    sscanf(pchr, "%lf", &trans[*ntrans][0]); 
                }
                else if (i==8)
                {
                    sscanf(&pchr[2], "%lf", &trans[*ntrans][1]);
                }
                pchr = strtok(NULL, " ");
                i+=1;
            }
            trans[*ntrans][2] = fwhm;
            *ntrans+=1;
        }
    }

    fclose(pfile);
}

double gauss(double x, double mu, double A, double fwhm)
{
    double pi = acos(-1.);
    double pos = 0.;
    double h = 0.;

    fwhm = fwhm / ( 2. * sqrt(2.) ) ;
    pos = ( x - mu ) / fwhm;
    h = A / fwhm / sqrt(pi);

    return h * exp(- pow(pos, 2.) );
}

void spectrum(int *npts, double pts[][CRD_SIZE], double xmin, double xmax, double xstp, int ntrans, double trans[][CRD_SIZE])
{
    int i, j;

    double evnm = 1241.25;
    double x = 0.;
    double sum = 0.;

    *npts=abs( (xmax - xmin) / xstp ) + 1;

    for (i=0; i<*npts; i++)
    {

        x = xmin + (double)i * xstp;
        sum = 0.;

        for (j=0; j<ntrans; j++)
        {
            sum+=gauss(evnm / x, trans[j][0], trans[j][1], trans[j][2]);
        }

        pts[i][0] = x;
        pts[i][1] = sum;
    }
}

void wspec(char *filename, int npts, double pts[][CRD_SIZE])
{
    int i;

    FILE *pfile = NULL;

    if (!strcmp(filename, "stdout"))
    {
        pfile = stdout;
    }
    else
    {
        pfile = fopen(filename, "w");
    }

    for (i=0; i<npts; i++)
    {
        fprintf(pfile, "%12.6lf%12.6lf\n", pts[i][0], pts[i][1]);
    }

    fclose(pfile);
}
