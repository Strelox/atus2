//
// ATUS2 - The ATUS2 package is atom interferometer Toolbox developed at ZARM
// (CENTER OF APPLIED SPACE TECHNOLOGY AND MICROGRAVITY), Germany. This project is
// founded by the DLR Agentur (Deutsche Luft und Raumfahrt Agentur). Grant numbers:
// 50WM0942, 50WM1042, 50WM1342.
// Copyright (C) 2017 Želimir Marojević, Ertan Göklü, Claus Lämmerzahl
//
// This file is part of ATUS2.
//
// ATUS2 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ATUS2 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ATUS2.  If not, see <http://www.gnu.org/licenses/>.
//

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <omp.h>
#include "anyoption.h"
#include "fftw3.h"
#include "my_structs.h"

extern double sign( const double val );

enum
{
  I_CONST = 0x01,
  J_CONST = 0x02,
  K_CONST = 0x04,
  RT = 0x08,
  IT = 0x10,
  BW = 0x20,
  PH = 0x80
};

using namespace std;

double mode1( double rt, double it )
{
  return rt;
}

double mode2( double rt, double it )
{
  return it;
}

double mode3( double rt, double it )
{
  return rt*rt+it*it;
}

double mode4( double rt, double it )
{
  return atan2(it,rt);
}

// 3D Fall
void display_3D( fftw_complex *field, generic_header *header, int opt, const int i0, const int j0, const int k0 )
{
  const int dimX = header->nDimX;
  const int dimY = header->nDimY;
  const int dimZ = header->nDimZ;
  const double dx = header->dx;
  const double dy = header->dy;
  const double dz = header->dz;
  const double x0 = header->xMin - header->dx;
  const double y0 = header->yMin - header->dy;
  const double z0 = header->zMin - header->dz;

  int i, j, k, ijk;
  double x, y, z;
  double (*fkt)(double, double);

  if ( opt & RT ) fkt = &mode1;
  else if ( opt & IT ) fkt = &mode2;
  else if ( opt & PH ) fkt = &mode4;
  else fkt = &mode3;

  if ( i0 < 0 || i0 > dimX ) opt &= ~I_CONST;
  if ( j0 < 0 || j0 > dimY ) opt &= ~J_CONST;
  if ( k0 < 0 || k0 > dimY ) opt &= ~K_CONST;

  if ( (opt & (I_CONST|J_CONST)) ==  (I_CONST|J_CONST) )
  {
    z = z0;
    for ( k=0; k<dimZ; k++ )
    {
      z += dz;
      ijk = k+dimZ*(j0+dimY*i0);
      printf( "%e\t%e\n", z, (*fkt)(field[ijk][0],field[ijk][1]) );
    }
    return;
  }

  if ( (opt & (I_CONST|K_CONST)) ==  (I_CONST|K_CONST) )
  {
    y = y0;
    for ( j=0; j<dimY; j++ )
    {
      y += dy;
      ijk = k0+dimZ*(j+dimY*i0);
      printf( "%e\t%e\n", y, (*fkt)(field[ijk][0],field[ijk][1]) );
    }
    return;
  }

  if ( (opt & (J_CONST|K_CONST)) ==  (J_CONST|K_CONST) )
  {
    x = x0;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      ijk = k0+dimZ*(j0+dimY*i);
      printf( "%e\t%e\n", x, (*fkt)(field[ijk][0],field[ijk][1]) );
    }
    return;
  }

  if ( opt & K_CONST )
  {
    x = x0;
    y = y0;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      y = y0;
      for ( j=0; j<dimY; j++ )
      {
        y += dy;
        ijk = k0+dimZ*(j+dimY*i);
        printf( "%e\t%e\t%e\n", x, y, (*fkt)(field[ijk][0],field[ijk][1]) );
      }
      printf( "\n" );
    }
    return;
  }

  if ( opt & J_CONST )
  {
    x = x0;
    z = z0;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      z = z0;
      for ( k=0; k<dimZ; k++ )
      {
        z += dz;
        ijk = k+dimZ*(j0+dimY*i);
        printf( "%e\t%e\t%e\n", x, z, (*fkt)(field[ijk][0],field[ijk][1]) );
      }
      printf( "\n" );
    }
    return;
  }

  if ( opt & I_CONST )
  {
    y = y0;
    z = z0;
    for ( j=0; j<dimY; j++ )
    {
      y += dy;
      z = z0;
      for ( k=0; k<dimZ; k++ )
      {
        z += dz;
        ijk = k+dimZ*(j+dimY*i0);
        printf( "%e\t%e\t%e\n", y, z, (*fkt)(field[ijk][0],field[ijk][1]) );
      }
      printf( "\n" );
    }
    return;
  }
}

// 3D Fall
void display_3D( double *field, generic_header *header, int opt, const int i0, const int j0, const int k0 )
{
  const int dimX = header->nDimX;
  const int dimY = header->nDimY;
  const int dimZ = header->nDimZ;
  const double dx = header->dx;
  const double dy = header->dy;
  const double dz = header->dz;
  const double x0 = header->xMin - header->dx;
  const double y0 = header->yMin - header->dy;
  const double z0 = header->zMin - header->dz;

  int i, j, k, ijk;
  double x, y, z;

  if ( i0 < 0 || i0 > dimX ) opt &= ~I_CONST;
  if ( j0 < 0 || j0 > dimY ) opt &= ~J_CONST;
  if ( k0 < 0 || k0 > dimY ) opt &= ~K_CONST;

  if ( (opt & (I_CONST|J_CONST)) ==  (I_CONST|J_CONST) )
  {
    z = z0;
    for ( k=0; k<dimZ; k++ )
    {
      z += dz;
      ijk = k+dimZ*(j0+dimY*i0);
      printf( "%e\t%e\n", z, field[ijk] );
    }
    return;
  }

  if ( (opt & (I_CONST|K_CONST)) ==  (I_CONST|K_CONST) )
  {
    y = y0;
    for ( j=0; j<dimY; j++ )
    {
      y += dy;
      ijk = k0+dimZ*(j+dimY*i0);
      printf( "%e\t%e\n", y, field[ijk] );
    }
    return;
  }

  if ( (opt & (J_CONST|K_CONST)) ==  (J_CONST|K_CONST) )
  {
    x = x0;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      ijk = k0+dimZ*(j0+dimY*i);
      printf( "%e\t%e\n", x, field[ijk] );
    }
    return;
  }

  if ( opt & K_CONST )
  {
    x = x0;
    y = y0;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      y = y0;
      for ( j=0; j<dimY; j++ )
      {
        y += dy;
        ijk = k0+dimZ*(j+dimY*i);
        printf( "%e\t%e\t%e\n", x, y, field[ijk] );
      }
      printf( "\n" );
    }
    return;
  }

  if ( opt & J_CONST )
  {
    x = x0;
    z = z0;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      z = z0;
      for ( k=0; k<dimZ; k++ )
      {
        z += dz;
        ijk = k+dimZ*(j0+dimY*i);
        printf( "%e\t%e\t%e\n", x, z, field[ijk] );
      }
      printf( "\n" );
    }
    return;
  }

  if ( opt & I_CONST )
  {
    y = y0;
    z = z0;
    for ( j=0; j<dimY; j++ )
    {
      y += dy;
      z = z0;
      for ( k=0; k<dimZ; k++ )
      {
        z += dz;
        ijk = k+dimZ*(j+dimY*i0);
        printf( "%e\t%e\t%e\n", y, z, field[ijk] );
      }
      printf( "\n" );
    }
    return;
  }
}

// 2D Fall
void display_2D( fftw_complex *field, generic_header *header, int opt, const int i0, const int j0 )
{
  const int    dimX = header->nDimX;
  const int    dimY = header->nDimY;
  const double dx   = header->dx;
  const double dy   = header->dy;
  const double x0   = header->xMin - header->dx;
  const double y0   = header->yMin - header->dy;

  int i, j, ij;
  double x, y;
  double (*fkt)(double, double);

  if ( i0 < 0 || i0 > dimX ) opt &= ~I_CONST;
  if ( j0 < 0 || j0 > dimY ) opt &= ~J_CONST;

  if ( opt & RT ) fkt = &mode1;
  else if ( opt & IT ) fkt = &mode2;
  else if ( opt & PH ) fkt = &mode4;
  else fkt = &mode3;

  if ( opt & I_CONST )
  {
    x = x0 + double(i0+1)*dx;
    y = y0;
    for ( j=0; j<dimY; j++ )
    {
      y += dy;
      ij = j+dimY*i0;
      printf( "%e\t%e\n", y, (*fkt)(field[ij][0],field[ij][1]) );
    }
    return;
  }

  if ( opt & J_CONST )
  {
    x = x0;
    y = y0 + double(j0+1)*dy;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      ij = j0+dimY*i;
      printf( "%e\t%e\n", x, (*fkt)(field[ij][0],field[ij][1]) );
    }
    return;
  }

  x = x0;
  for ( i=0; i<dimX; i++ )
  {
    x += dx;
    y  = y0;
    for ( j=0; j<dimY; j++ )
    {
      y += dy;
      ij = j+dimY*i;

      printf( "%g\t%g\t%.10e\n", x, y, (*fkt)(field[ij][0],field[ij][1]) );
    }
    printf( "\n" );
  }
}

// 2D Fall
void display_2D( double *field, generic_header *header, int opt, const int i0, const int j0 )
{
  const int dimX = header->nDimX;
  const int dimY = header->nDimY;
  const double dx = header->dx;
  const double dy = header->dy;
  const double x0 = header->xMin - header->dx;
  const double y0 = header->yMin - header->dy;

  int i, j, ij;
  double x, y;

  if ( i0 < 0 || i0 > dimX ) opt &= ~I_CONST;
  if ( j0 < 0 || j0 > dimY ) opt &= ~J_CONST;

  if ( opt & I_CONST )
  {
    x = x0 + double(i0+1)*dx;
    y = y0;
    for ( j=0; j<dimY; j++ )
    {
      y += dy;
      ij = j+dimY*i0;
      printf( "%e\t%e\n", y, field[ij] );
    }
    return;
  }

  if ( opt & J_CONST )
  {
    x = x0;
    y = y0 + double(j0+1)*dy;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      ij = j0+dimY*i;
      printf( "%e\t%e\n", x, field[ij] );
    }
    return;
  }


  if ( opt & BW )
  {
    x = x0;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      y  = y0;
      for ( j=0; j<dimY; j++ )
      {
        y += dy;
        ij = j+dimY*i;

        printf( "%e\t%e\t%e\n", x, y, sign(field[ij]) );
      }
      printf( "\n" );
    }
  }
  else
  {
    x = x0;
    for ( i=0; i<dimX; i++ )
    {
      x += dx;
      y  = y0;
      for ( j=0; j<dimY; j++ )
      {
        y += dy;
        ij = j+dimY*i;

        printf( "%g\t%g\t%.10e\n", x, y, field[ij] );
      }
      printf( "\n" );
    }
  }
}

// 1D Fall
void display_1D( fftw_complex *field, generic_header *header )
{
  const int    dimX = header->nDimX;
  const double dx   = header->dx;
  double x    = header->xMin - header->dx;
  double rt   = 0.0;
  double it   = 0.0;

  for ( int i=0; i<dimX; i++ )
  {
    x += dx;
    rt = field[i][0];
    it = field[i][1];
    printf( "%e\t%e\t%e\t%e\t%e\n", x, rt*rt+it*it, rt, it, atan2(it,rt) );
  }
}

// 1D Fall
void display_1D( double *field, generic_header *header )
{
  const int    dimX = header->nDimX;
  const double dx   = header->dx;
  double x    = header->xMin - header->dx;

  for ( int i=0; i<dimX; i++ )
  {
    x += dx;
    printf( "%e\t%e\n", x, field[i] );
  }
}

int main(int argc, char *argv[])
{
  int i0=-1, j0=-1, k0=-1, options=0;

  string filename;

  AnyOption *opt = new AnyOption();

  opt->noPOSIX();
  //opt->setVerbose();
  //opt->autoUsagePrint(true);

  opt->addUsage( "" );
  opt->addUsage( "Usage: gpo2 [options] filename" );
  opt->addUsage( "" );
  opt->addUsage( " -h --help	Prints this help " );
  opt->addUsage( " -i val  	Fix x coord. index i at val" );
  opt->addUsage( " -j val  	Fix y coord. index j at val" );
  opt->addUsage( " -k val  	Fix z coord. index k at val" );
  opt->addUsage( " -re		prints out the real part of a complex wavefunction" );
  opt->addUsage( " -im		prints out the imag part of a complex wavefunction" );
  opt->addUsage( " -bw		" );
  opt->addUsage( " -ph		" );
  opt->addUsage( "" );

  opt->setFlag(  "help", 'h' );
  opt->setOption( "i" );
  opt->setOption( "j" );
  opt->setOption( "k" );
  opt->setFlag(  "re" );
  opt->setFlag(  "im" );
  opt->setFlag(  "bw" );
  opt->setFlag(  "ph" );

  opt->processCommandArgs( argc, argv );

  if ( opt->getFlag( "help" ) || opt->getFlag( 'h' ) ) opt->printUsage();

  if ( opt->getValue("i") != nullptr )
  {
    i0 = atof(opt->getValue("i"));
    options |= I_CONST;
  };
  if ( opt->getValue("j") != nullptr )
  {
    j0 = atof(opt->getValue("j"));
    options |= J_CONST;
  };
  if ( opt->getValue("k") != nullptr )
  {
    k0 = atof(opt->getValue("k"));
    options |= K_CONST;
  };

  if ( opt->getFlag( "re" ) ) options |= RT;
  if ( opt->getFlag( "im" ) ) options |= IT;
  if ( opt->getFlag( "bw" ) ) options |= BW;
  if ( opt->getFlag( "ph" ) ) options |= PH;


  if ( opt->getArgc() != 0 ) filename = opt->getArgv(0);
  else opt->printUsage();

  delete opt;

  generic_header header;

  ifstream in;
  in.open(filename.c_str());
  if ( !in.is_open() ) return EXIT_FAILURE;
  in.read( (char *)&header, sizeof(generic_header) );

  int no_of_threads = 4;
  char *envstr = getenv( "MY_NO_OF_THREADS" );
  if ( envstr != nullptr ) no_of_threads = atoi( envstr );
  omp_set_num_threads( no_of_threads );

  printf( "### %s\n", filename.c_str() );
  printf( "# nDims    == %lld\n", header.nDims );
  printf( "# nDimX    == %lld\n", header.nDimX );
  printf( "# nDimY    == %lld\n", header.nDimY );
  printf( "# nDimZ    == %lld\n", header.nDimZ );
  printf( "# nDatatyp == %lld\n", header.nDatatyp );
  printf( "# bAtom    == %d\n", header.bAtom );
  printf( "# bComplex == %d\n", header.bComplex );
  printf( "# t        == %g\n", header.t );
  printf( "# dt       == %g\n", header.dt );
  printf( "# xMin     == %g\n", header.xMin );
  printf( "# xMax     == %g\n", header.xMax );
  printf( "# yMin     == %g\n", header.yMin );
  printf( "# yMax     == %g\n", header.yMax );
  printf( "# zMin     == %g\n", header.zMin );
  printf( "# zMax     == %g\n", header.zMax );
  printf( "# dx       == %g\n", header.dx );
  printf( "# dy       == %g\n", header.dy );
  printf( "# dz       == %g\n", header.dz );
  printf( "# dkx      == %g\n", header.dkx );
  printf( "# dky      == %g\n", header.dky );
  printf( "# dkz      == %g\n", header.dkz );
  printf( "# i0       == %d\n", i0 );
  printf( "# j0       == %d\n", j0 );
  printf( "# k0       == %d\n", k0 );

  size_t total_no_bytes;

  // 1D Stuff
  if ( header.nDims == 1 && header.bComplex == 1 )
  {
    total_no_bytes = sizeof(fftw_complex)*header.nDimX;
    fftw_complex *field = fftw_alloc_complex( header.nDimX );
    in.read( (char *)field, total_no_bytes );

    display_1D( field, &header );

    fftw_free(field);
  }

  // 1D Stuff
  if ( header.nDims == 1 && header.bComplex == 0 )
  {
    total_no_bytes = sizeof(double)*header.nDimX;
    double *field = fftw_alloc_real( header.nDimX );
    in.read( (char *)field, total_no_bytes );

    display_1D( field, &header );

    fftw_free(field);
  }

  // 2D Stuff
  if ( header.nDims == 2 && header.bComplex == 1 )
  {
    total_no_bytes = sizeof(fftw_complex)*header.nDimX*header.nDimY;

    fftw_complex *field = fftw_alloc_complex(header.nDimX*header.nDimY);
    in.read( (char *)field, total_no_bytes );

    display_2D( field, &header, options, i0, j0 );

    fftw_free(field);
  }

  // 2D Stuff
  if ( header.nDims == 2 && header.bComplex == 0 )
  {
    total_no_bytes = sizeof(double)*header.nDimX*header.nDimY;
    double *field = fftw_alloc_real(header.nDimX*header.nDimY);
    in.read( (char *)field, total_no_bytes );

    display_2D( field, &header, options, i0, j0 );

    fftw_free(field);
  }

  if ( header.nDims == 3 && header.bComplex == 1  )
  {
    total_no_bytes = sizeof(fftw_complex)*header.nDimX*header.nDimY*header.nDimZ;

    fftw_complex *field = fftw_alloc_complex(header.nDimX*header.nDimY*header.nDimZ);
    in.read( (char *)field, total_no_bytes );

    display_3D( field, &header, options, i0, j0, k0 );

    fftw_free(field);
  }

  if ( header.nDims == 3 && header.bComplex == 0  )
  {
    total_no_bytes = sizeof(double)*header.nDimX*header.nDimY*header.nDimZ;

    double *field = fftw_alloc_real(header.nDimX*header.nDimY*header.nDimZ);
    in.read( (char *)field, total_no_bytes );

    display_3D( field, &header, options, i0, j0, k0 );

    fftw_free(field);
  }
}
