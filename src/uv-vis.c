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

    rtrans(argv[1], &ntrans, trans, fwhm);

    printf(" %s: %d transitions\n", argv[1], ntrans);

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
    wspec("out", npts, pts);    
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
        exit(0);
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

double gauss(double x, double mu, double A, double fwhm)
{
    double pi = acos(-1.);
    double pos = 0.;
    double h = 0.;

    fwhm = fwhm / ( 2. * sqrt(2.) ) ;
    pos = ( x - mu ) / fwhm;
    h = A / fwhm / sqrt(pi);

    printf("%lf\n", fwhm);
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
    pfile = fopen(filename, "w");

    for (i=0; i<npts; i++)
    {
        fprintf(pfile, "%12.6lf%12.6lf\n", pts[i][0], pts[i][1]);
    }

    fclose(pfile);
}
