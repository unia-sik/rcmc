// Completely rewritten
//
// Difference to original
// - Simplified main loop and removed psium and psilm, because they
//   are never used
// - Maximum iteration in multigrid solver loop


#define MAX_THREADS 32
// #include "harness.h"
#include "mpi.h"
#include "stdbool.h"
#include "math.h"


#ifndef DEFAULT_LOG2_P
#define DEFAULT_LOG2_P 2
#endif
#ifndef DEFAULT_LOG2_N
#define DEFAULT_LOG2_N 3
#endif


#define MAX_NUMLEV 16


#define BLACK   1
#define RED     2


#define MB_LEVTOL       1
#define MB_F            2
#define MB_ROW          3
#define MB_COL          4
#define MB_GA           5
#define MB_GB           6
#define MB_PSIB         7
#define MB_W2           8
#define MB_TAUZ         9

// two entries each:
#define MB_PSI          10
#define MB_PSIM         12
#define MB_W1           14
#define MB_W5           16
#define MB_W7           18
#define MB_GAMMA        20

// MAX_NUMLEV entries each:
#define MB_Q            22
#define MB_RHS          (22+MAX_NUMLEV)




#define DIR_N           0
#define DIR_S           (DIR_N|1)
#define DIR_W           2
#define DIR_E           (DIR_W|1)
#define DIR_NW          4
#define DIR_SE          (DIR_NW|1)
#define DIR_NE          6
#define DIR_SW          (DIR_NE|1)

#define COL_ALL         0


// data that is send to every core
typedef struct params_data_s {
    int log2_grid; // grid_width = 2**log2_grid + 2
    int log2_nprocs;
    long numlev;

    double res;
    double dtau;
    double tolerance;

    // constants
    double h1;
    double h3;
    double h0;
    double lf;
    double f0;
    double beta;
    double gpr;
    double t0;
    double time_interval; // in seconds
    uint_fast64_t maxwork;
} params_data_t;





typedef struct percore_data_s {
    // individual per core, but fixed (only depending on core id)
    long rownum;
    long colnum;

    params_data_t params;

    bool border_n;
    bool border_s;
    bool border_w;
    bool border_e;

    // pre-calculated tables, once written at beginning of each thread
    // identical on every core
    double* f;
    double eig2;
    double psibi;

    // only locally used (by owning core)
    double* lev_tol;
    double* one_row;
    double* one_col;
    double** ga;
    double** gb;
    double** gamma[2];
    double** work5[2];
    double** tauz;
    double** psi[2];
    double** psim[2];
    double** work1[2];
    double** work7[2];
    double** psib;
    double** work2;
    double** q[MAX_NUMLEV];
    double** rhs[MAX_NUMLEV];

    mpi_communicator_t c_all;
} percore_data_t;








/* Shared memory implementation of the multigrid method
   Implementation uses red-black gauss-seidel relaxation
   iterations, w cycles, and the method of half-injection for
   residual computation. */





// -----------------------------------------------------------------------------
// Functions that only work on the input parameters, no further memory accesses
// -----------------------------------------------------------------------------


// clear edges after Jacobi or Laplace transformation, if there are no
// neigbours
// BUGFIX: don't forget corners
void clear_edges(double** z, int lastrow, int lastcol, double val,
                 bool n, bool s, bool w, bool e)
{
    int j;

    if (n) {
        for (j = 0; j <= lastcol + 1; j++) z[0][j] = val;
    }

    if (s) {
        for (j = 0; j <= lastcol + 1; j++) z[lastrow + 1][j] = val;
    }

    if (w) {
        for (j = 0; j <= lastrow + 1; j++) z[j][0] = val;
    }

    if (e) {
        for (j = 0; j <= lastrow + 1; j++) z[j][lastcol + 1] = val;
    }
}


// nearest-neigbour Jacobi computation with 9-point Moore neigbourhood
void jacobian(double** result, double** x, double** y, double factor,
              int lastrow, int lastcol)
{
    int i, j;

    for (i = 1; i <= lastrow; i++) {
        for (j = 1; j <= lastcol; j++) {
            result[i][j] = factor *
                           ((y[i  ][j - 1] + y[i + 1][j - 1] - y[i  ][j + 1] - y[i + 1][j + 1]) *
                            (x[i + 1][j  ] - x[i  ][j  ])
                            + (y[i - 1][j - 1] + y[i  ][j - 1] - y[i - 1][j + 1] - y[i  ][j + 1]) *
                            (x[i  ][j  ] - x[i - 1][j  ])
                            + (y[i + 1][j  ] + y[i + 1][j + 1] - y[i - 1][j  ] - y[i - 1][j + 1]) *
                            (x[i  ][j + 1] - x[i  ][j  ])
                            + (y[i + 1][j - 1] + y[i + 1][j  ] - y[i - 1][j - 1] - y[i - 1][j  ]) *
                            (x[i  ][j  ] - x[i  ][j - 1])
                            + (y[i + 1][j  ] - y[i  ][j + 1]) * (x[i + 1][j + 1] - x[i  ][j  ])
                            + (y[i  ][j - 1] - y[i - 1][j  ]) * (x[i  ][j  ] - x[i - 1][j - 1])
                            + (y[i  ][j + 1] - y[i - 1][j  ]) * (x[i - 1][j + 1] - x[i  ][j  ])
                            + (y[i + 1][j  ] - y[i  ][j - 1]) * (x[i  ][j  ] - x[i + 1][j - 1]));
        }
    }
}


// calculate the 5-point stencil Laplacian of a subblock x
void laplacian(double** result, double** x, double factor,
               int lastrow, int lastcol)
{
    int i, j;

    for (i = 1; i <= lastrow; i++) {
        for (j = 1; j <= lastcol; j++) {
            result[i][j] = factor *
                           (x[i + 1][j] + x[i - 1][j] + x[i][j + 1] + x[i][j - 1] - 4.0 * x[i][j]);
        }
    }
}


double relax(double** q, double** rhs,  int lastrow, int lastcol,
             unsigned redblack_row, unsigned redblack_col, double inv_factor, double maxerr)
{
    int i, j;

    for (i = redblack_row; i <= lastrow; i += 2) {
        for (j = redblack_col; j <= lastcol; j += 2) {
            double val = (q[i][j + 1] + q[i][j - 1] + q[i - 1][j] + q[i + 1][j]
                          - rhs[i][j]) * inv_factor;
            double err = q[i][j] - val;

            if (err > maxerr) maxerr = err;
            else if (-err > maxerr) maxerr = -err; // avoid fabs

            q[i][j] = val;
        }
    }

    return maxerr;
}


// go to next finest grid by interpolation and addition
void interpolation(double** finer, double** q, int lastrow, int lastcol,
                   int i_off, int j_off, int nk)
{
    int i, j;
    double coeff_k1 = 1.0 / ((1 << (nk + 1)) + 1);
    int imx = (1 << (nk)) + 1;

    for (i = 1; i <= lastrow; i++) {
        double ci1 = (imx - (i + i_off)) * coeff_k1;
        double ci2 = (i + i_off) * coeff_k1;

        for (j = 1; j <= lastcol; j++) {
            double cj1 = (imx - (j + j_off)) * coeff_k1;
            double cj2 = (j + j_off) * coeff_k1;
            double d1 = cj1 * q[i  ][j - 1] + (1.0 - cj1) * q[i  ][j];
            double d2 = cj1 * q[i - 1][j - 1] + (1.0 - cj1) * q[i - 1][j];
            double d3 = cj1 * q[i + 1][j - 1] + (1.0 - cj1) * q[i + 1][j];
            double d4 = cj2 * q[i  ][j + 1] + (1.0 - cj2) * q[i  ][j];
            double d5 = cj2 * q[i - 1][j + 1] + (1.0 - cj2) * q[i - 1][j];
            double d6 = cj2 * q[i + 1][j + 1] + (1.0 - cj2) * q[i + 1][j];

            finer[2 * i - 1][2 * j - 1] += ci1 * d2 + (1.0 - ci1) * d1;
            finer[2 * i  ][2 * j - 1] += ci2 * d3 + (1.0 - ci2) * d1;
            finer[2 * i - 1][2 * j]   += ci1 * d5 + (1.0 - ci1) * d4;
            finer[2 * i  ][2 * j]   += ci2 * d6 + (1.0 - ci2) * d4;
        }
    }
}


double calc_rhs(bool border, double** rhs, double** q,
                double factor, double wi, double wj, int i, int j,
                double l0, double l1, double u0, double u1)
{
    double s1 = rhs[2 * i][2 * j]
                - q[2 * i][2 * j + 1]
                - q[2 * i][2 * j - 1]
                - q[2 * i - 1][2 * j]
                - q[2 * i + 1][2 * j]
                + factor * q[2 * i][2 * j];

    if (border) return (1.0 - wi) * (2.0 - wj) * s1;

    double s2 = rhs[2 * i][2 * j - 2]
                - q[2 * i][2 * j - 1]
                - q[2 * i - 1][2 * j - 2]
                - q[2 * i + 1][2 * j - 2]
                - l1 // - q[2*i][2*j-3]
                + q[2 * i][2 * j - 2] * factor;
    double s3 = rhs[2 * i - 2][2 * j]
                - q[2 * i - 2][2 * j - 1]
                - q[2 * i - 1][2 * j]
                - q[2 * i - 2][2 * j + 1]
                - u1 // - q[2*i-3][2*j]
                + q[2 * i - 2][2 * j] * factor;
    double s4 = rhs[2 * i - 2][2 * j - 2]
                - q[2 * i - 2][2 * j - 1]
                - q[2 * i - 1][2 * j - 2]
                - l0 // - q[2*i-2][2*j-3]
                - u0 // - q[2*i-3][2*j-2]
                + q[2 * i - 2][2 * j - 2] * factor;
    return wi * (wj * s4 + (2.0 - wj) * s3) + (1.0 - wi) * (wj * s2 + (2.0 - wj) * s1);
}



//NOTE this function uses the fgmp-functions and not directly MPI (because of the stride!)
void forward_into_offset(int offset, int count, double* send_buffer, int send_stride, double* recv_buffer, int recv_stride, percore_data_t* percore)
{
    int x = fgmp_addr_x((&percore->c_all)->address);
    int y = fgmp_addr_y((&percore->c_all)->address);

    int dx = offset % (&percore->c_all)->width;
    int dy = offset / (&percore->c_all)->width;

    if ((x - dx) >= 0 && (y - dy) >= 0 && (x - dx) < (&percore->c_all)->width && (y - dy) < (&percore->c_all)->height) {
        if ((x + dx) >= 0 && (y + dy) >= 0 && (x + dx) < (&percore->c_all)->width && (y + dy) < (&percore->c_all)->height) {
            fgmp_srdy(fgmp_addr_gen(x - dx, y - dy));

            for (int i = 0; i < count; i++) {
                fgmp_bsf();
                fgmp_snd(fgmp_addr_gen(x + dx, y + dy), send_buffer[i * send_stride]);
                fgmp_bre();
                recv_buffer[i * recv_stride] = fgmp_rcvp();
            }
        } else {
            fgmp_srdy(fgmp_addr_gen(x - dx, y - dy));

            for (int i = 0; i < count; i++) {
                fgmp_bre();
                recv_buffer[i * recv_stride] = fgmp_rcvp();
            }
        }
    } else {
        if ((x + dx) >= 0 && (y + dy) >= 0 && (x + dx) < (&percore->c_all)->width && (y + dy) < (&percore->c_all)->height) {
            for (int i = 0; i < count; i++) {
                fgmp_bsf();
                fgmp_snd(fgmp_addr_gen(x + dx, y + dy), send_buffer[i * send_stride]);
            }
        } else {
            //do nothing
        }
    }
}


// -----------------------------------------------------------------------------
// Functions to forward data between nodes
// -----------------------------------------------------------------------------


#define YX(y, x) (base+(y)*(xm+2)+(x))

void forward_laplacian_edges(void* m, int xm,  int ym, percore_data_t* percore)
//                        memblock, lastcol, lastrow
{
    int base = (ym + 2) * sizeof(double*) / sizeof(double);


    MPI_Barrier((&percore->c_all));
    forward_into_offset(-((&percore->c_all)->width), xm, (double*)m + YX(0,    1), 1, (double*)m + YX(ym, 1), 1, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(+((&percore->c_all)->width), xm, (double*)m + YX(ym + 1, 1), 1, (double*)m + YX(1,  1), 1, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(1, ym, (double*)m + YX(1, 0), xm + 2, (double*)m + YX(1,  xm), xm + 2, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(-1, ym, (double*)m + YX(1, xm + 1), xm + 2, (double*)m + YX(1,  1), xm + 2, percore);


//     parop_forward_begin();
//     parop_forward_double(DIR_S, xm, m, YX(0,    1), 1,    m, YX(ym, 1), 1);
//     parop_forward_double(DIR_N, xm, m, YX(ym+1, 1), 1,    m, YX(1,  1), 1);
//     parop_forward_double(DIR_E, ym, m, YX(1,    0), xm+2, m, YX(1, xm), xm+2);
//     parop_forward_double(DIR_W, ym, m, YX(1, xm+1), xm+2, m, YX(1,  1), xm+2);
//     parop_forward_end();
}


void forward_edges_and_corners(void* m, int xm,  int ym, percore_data_t* percore)
//                          memblock, lastcol, lastrow
{
    // The vertical (N,S) and horizontal (W,E) borders overlap at the corners.
    // Therefore the corner values are written twice and their values depend
    // on the order of the write accesses. But that is not a problem, since in
    // the case of double writing, a vertical as well as a horizontal neighbour
    // exists and therefore we do not take any of the two values but overwrite
    // it by the value from the diagonal neighbour (see later). If only one of
    // both neighbours exists, only the respective value is written. And if
    // no neighbour exists, the value is unchanged.

    int base = (ym + 2) * sizeof(double*) / sizeof(double);
//     parop_forward_begin();

    MPI_Barrier((&percore->c_all));
    forward_into_offset(-((&percore->c_all)->width), xm + 2, (double*)m + YX(0,    1), 1, (double*)m + YX(ym, 0), 1, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(+((&percore->c_all)->width), xm + 2, (double*)m + YX(ym + 1, 1), 1, (double*)m + YX(1,  0), 1, percore);

    MPI_Barrier((&percore->c_all));
    forward_into_offset(1, ym + 2, (double*)m + YX(0, 0), xm + 2, (double*)m + YX(0,  xm), xm + 2, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(-1, ym + 2, (double*)m + YX(1, xm + 1), xm + 2, (double*)m + YX(0,  1), xm + 2, percore);
    // edges
//     parop_forward_double(DIR_S, xm+2, m, YX(0,    0), 1,    m, YX(ym, 0), 1);
//     parop_forward_double(DIR_N, xm+2, m, YX(ym+1, 0), 1,    m, YX(1,  0), 1);
//     parop_forward_double(DIR_E, ym+2, m, YX(0,    0), xm+2, m, YX(0, xm), xm+2);
//     parop_forward_double(DIR_W, ym+2, m, YX(0, xm+1), xm+2, m, YX(0,  1), xm+2);

    MPI_Barrier((&percore->c_all));
    forward_into_offset(-((&percore->c_all)->width) + 1, 1, (double*)m + YX(0,       0), 1, (double*)m + YX(ym, xm), 1, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(-((&percore->c_all)->width) - 1, 1, (double*)m + YX(0,    xm + 1), 1, (double*)m + YX(ym,  1), 1, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(+((&percore->c_all)->width) + 1, 1, (double*)m + YX(ym + 1,    0), 1, (double*)m + YX(1,  xm), 1, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(+((&percore->c_all)->width) - 1, 1, (double*)m + YX(ym + 1, xm + 1), 1, (double*)m + YX(1,   1), 1, percore);

    // corners
//     parop_forward_double(DIR_SE, 1, m, YX(0,       0), 1, m, YX(ym, xm), 1);
//     parop_forward_double(DIR_SW, 1, m, YX(0,    xm+1), 1, m, YX(ym,  1), 1);
//     parop_forward_double(DIR_NE, 1, m, YX(ym+1,    0), 1, m, YX(1,  xm), 1);
//     parop_forward_double(DIR_NW, 1, m, YX(ym+1, xm+1), 1, m, YX(1,   1), 1);

//     parop_forward_end();
}


void copy_rhs_borders(percore_data_t* percore, int k, int xm, int ym)
{
    int base = (ym + 2) * sizeof(double*) / sizeof(double);

    MPI_Barrier((&percore->c_all));
    forward_into_offset(-((&percore->c_all)->width) + 1, 1, (double*)percore->rhs[k] + YX(0,       0), 1, (double*)percore->rhs[k]  + YX(ym, xm), 1, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(-((&percore->c_all)->width)    , xm / 2, (double*)percore->rhs[k] + YX(0,       2), 2, (double*)percore->rhs[k]  + YX(ym, 2), 2, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(1,                            ym / 2, (double*)percore->rhs[k] + YX(2,       0), 2 * (xm + 2), (double*)percore->rhs[k]  + YX(2, xm), 2 * (xm + 2), percore);

    MPI_Barrier((&percore->c_all));
    forward_into_offset(-((&percore->c_all)->width)    , xm / 2 + 1, (double*)percore->one_row + 0, 1, (double*)percore->q[k]  + YX(ym - 1, 2), 2, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(1,                            ym / 2 + 1, (double*)percore->one_col + 0, 1, (double*)percore->q[k]  + YX(0, xm - 1), 2 * (xm + 2), percore);

//     parop_forward_begin();
//
//     parop_forward_double(DIR_SE, 1,
//         MB_RHS+k, YX(0, 0), 1,
//         MB_RHS+k, YX(ym, xm), 1);
//     parop_forward_double(DIR_S, xm/2,
//         MB_RHS+k, YX(0, 2), 2,
//         MB_RHS+k, YX(ym, 2), 2);
//     parop_forward_double(DIR_E, ym/2,
//         MB_RHS+k, YX(2, 0), 2*(xm+2),
//         MB_RHS+k, YX(2, xm), 2*(xm+2));
//
//     // get west column and north row
//     parop_forward_double(DIR_S, xm/2+1,
//         MB_ROW, 0, 1,
//         MB_Q+k, YX(ym-1, 0), 2);
//     parop_forward_double(DIR_E, ym/2+1,
//         MB_COL, 0, 1,
//         MB_Q+k, YX(0, xm-1), 2*(xm+2));
//
//     parop_forward_end();
}


void forward_redblack(void* m, unsigned rb, int xm, int ym, percore_data_t* percore)
{
    int base = (ym + 2) * sizeof(double*) / sizeof(double);

    MPI_Barrier((&percore->c_all));
    forward_into_offset(-((&percore->c_all)->width), xm / 2, (double*)m + YX(0,    rb), 2, (double*)m + YX(ym, rb), 2, percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(+((&percore->c_all)->width), xm / 2, (double*)m + YX(ym + 1, 3 - rb), 2, (double*)m + YX(1,  3 - rb), 2, percore);

    MPI_Barrier((&percore->c_all));
    forward_into_offset(1, ym / 2, (double*)m + YX(rb, 0), 2 * (xm + 2), (double*)m + YX(0,  xm), 2 * (xm + 2), percore);
    MPI_Barrier((&percore->c_all));
    forward_into_offset(-1, ym / 2, (double*)m + YX(3 - rb, xm + 1), 2 * (xm + 2), (double*)m + YX(3 - rb,  1), 2 * (xm + 2), percore);
//     parop_forward_begin();
//
//     parop_forward_double(DIR_S, xm/2,
//         memblock, YX(0,      rb), 2,
//         memblock, YX(ym,     rb), 2);
//     parop_forward_double(DIR_N, xm/2,
//         memblock, YX(ym+1, 3-rb), 2,
//         memblock, YX(1,    3-rb), 2);
//     parop_forward_double(DIR_E, ym/2,
//         memblock, YX(rb,      0), 2*(xm+2),
//         memblock, YX(rb,     xm), 2*(xm+2));
//     parop_forward_double(DIR_W, ym/2,
//         memblock, YX(3-rb, xm+1), 2*(xm+2),
//         memblock, YX(3-rb,    1), 2*(xm+2));
//
//     parop_forward_end();
}







// -----------------------------------------------------------------------------
// Complex functions on the node context
// -----------------------------------------------------------------------------



double multig_factor(percore_data_t* percore, long k)
{
    return 4.0 - percore->eig2 * percore->params.res *
           percore->params.res * (1LL << (2 * (percore->params.numlev - 1 - k)));
}


/* perform half-injection to next coarsest level                */
void rescal(percore_data_t* percore, long k)
{
    int i, j;
    int ihi = 1 << (k);
    int jhi = 1 << ((percore->params.log2_nprocs & 1) + k);
    int i_off = percore->rownum * ihi;
    int j_off = percore->colnum * jhi;
    double** q = percore->q[k];
    double** rhs = percore->rhs[k];
    double** coarse = percore->rhs[k - 1];
    double* col = percore->one_col;
    double* row = percore->one_row;
    bool border_n = percore->border_n;
    bool border_w = percore->border_w;
    double coeff_k1 = 1.0 / ((1 << (k + ((percore->params.log2_nprocs + 1) >> 1))) + 1);
    double factor = multig_factor(percore, k);

    double wi1 = 0.5 * (1 + i_off) * coeff_k1;
    double wj1 = (1 + j_off) * coeff_k1;

    // i=1 j=1
    coarse[1][1] = calc_rhs(border_n || border_w, rhs, q, factor,
                            wi1, wj1,
                            1, 1,
                            col[0],
                            col[1],
                            row[0],
                            row[1]);

    // i=1
    for (j = 2; j <= jhi; j++) {
        double wj = (j + j_off) * coeff_k1;
        coarse[1][j] = calc_rhs(border_n, rhs, q, factor,
                                wi1, wj,
                                1, j,
                                q[2 * 1 - 2][2 * j - 3],
                                q[2 * 1][2 * j - 3],
                                row[j - 1],
                                row[j]);
    }

    // j=1
    for (i = 2; i <= ihi; i++) {
        double wi = 0.5 * (i + i_off) * coeff_k1;
        coarse[i][1] = calc_rhs(border_w, rhs, q, factor,
                                wi, wj1,
                                i, 1,
                                col[i - 1],
                                col[i],
                                q[2 * i - 3][2 * 1 - 2],
                                q[2 * i - 3][2 * 1]);
    }

    for (i = 2; i <= ihi; i++) {
        double wi = 0.5 * (i + i_off) * coeff_k1;

        for (j = 2; j <= jhi; j++) {
            double wj = (j + j_off) * coeff_k1;
            coarse[i][j] = calc_rhs(false, rhs, q, factor, wi, wj, i, j,
                                    q[2 * i - 2][2 * j - 3],
                                    q[2 * i][2 * j - 3],
                                    q[2 * i - 3][2 * j - 2],
                                    q[2 * i - 3][2 * j]);
        }
    }
}


double calc_residual(percore_data_t* percore, int k)
{
    int lastrow = 1 << (k + 1);
    int lastcol = 1 << (k + 1 + (percore->params.log2_nprocs & 1));
    double reciprocal = 1.0 / multig_factor(percore, k);

    forward_redblack(percore->q[k], BLACK, lastcol, lastrow, percore);
    double residual = relax(percore->q[k], percore->rhs[k],
                            lastrow, lastcol, RED, RED, reciprocal, 0.0);
    residual = relax(percore->q[k], percore->rhs[k],
                     lastrow, lastcol, BLACK, BLACK, reciprocal, residual);

    forward_redblack(percore->q[k], RED, lastcol, lastrow, percore);
    residual = relax(percore->q[k], percore->rhs[k],
                     lastrow, lastcol, RED, BLACK, reciprocal, residual);
    residual = relax(percore->q[k], percore->rhs[k],
                     lastrow, lastcol, BLACK, RED, reciprocal, residual);

    double result;
    MPI_Allreduce(&residual, &result, 1, MPI_DOUBLE, MPI_MAX, (&percore->c_all));
//     return parop_allreduce_double(0, PAROP_OP_MAX, residual);
    return result;
}


// go to finer grid
void finer(percore_data_t* percore, int k)
{
    int lastrow = 1 << (k + 1);
    int lastcol = 1 << (k + 1 + (percore->params.log2_nprocs & 1));

    forward_edges_and_corners(percore->q[k], lastcol, lastrow, percore);

    interpolation(
        percore->q[k + 1],
        percore->q[k],
        lastrow,
        lastcol,
        percore->rownum * lastrow,
        percore->colnum * lastcol,
        ((percore->params.log2_nprocs + 3) >> 1) + k);
}


// go to coarser grid
void coarser(percore_data_t* percore, int k)
{
    int lastrow = 1 << (k + 1);
    int lastcol = 1 << (k + 1 + (percore->params.log2_nprocs & 1));

    forward_edges_and_corners(percore->q[k], lastcol, lastrow, percore);

    copy_rhs_borders(percore, k, lastcol, lastrow);
    rescal(percore, k);

    // Initial guess on coarser grid is all zero
    int i, j;
    int ihi = (1 << (k)) + 2;
    int jhi = (1 << (k + (percore->params.log2_nprocs & 1))) + 2;

    for (i = 0; i < ihi; i++)
        for (j = 0; j < jhi; j++)
            percore->q[k - 1][i][j] = 0.0;
}


#define MAX_RESIDUAL 0.6e30
#define MAX_ITER  10000

void multigrid_solver(percore_data_t* percore)
{
    int iter;
    int k = percore->params.numlev - 1; // start at finest grid
    uint_fast64_t work = 0;
    double improve_by_40 = MAX_RESIDUAL;
    double residual;

    for (iter = 1; iter <= MAX_ITER; iter++) {

        // Estimate the work:
        // A single relaxation sweep at the finest level is one unit of work
        // Stop the computation if work limit is exceeded.
        work += 1 << (2 * k);
//         if (work >= percore->params.maxwork)
//             harness_fatal("Maximum work limit exceeded");

        residual = calc_residual(percore, k);

        if (residual < percore->lev_tol[k]) {
            // If the residual is small enough, we converged and can continue
            // with the next finer grid.
            // If already at finest grid, we are done.
            if (k == percore->params.numlev - 1) break;

            finer(percore, k);
            k++;
            improve_by_40 = MAX_RESIDUAL;

        } else if ((residual > improve_by_40) && (k != 0)) {
            // Go to coarser grid, if the last improvement of the residual
            // was less than 40% and we have not reached the was coarsest grid
            coarser(percore, k);
            percore->lev_tol[k - 1] = 0.3 * residual;
            k--;
            improve_by_40 = MAX_RESIDUAL;

        } else {
            // Stay at this grid size, as long as the residual improves by at
            // least 40% or if we reached the coarsest grid size
            improve_by_40 = 0.6 * residual;
        }
    }


//     if (parop_my_rank == 0) {
//         harness_print_string("iter ");
//         harness_print_int32(iter);
// //        harness_print_string(", level ");
// //        harness_print_int32(k);
//         harness_print_string(", residual norm = ");
//         harness_print_int32(residual*1e14);
//         harness_print_string("e-14, work = ");
//         harness_print_int32((work<<12)>>(2*percore->params.numlev));
//         harness_print_string("\n");
//     }
}



void set_last_level(percore_data_t* percore, double** gamma, double ressqr)
{
    int lev = percore->params.numlev - 1;
    int lastrow = 1 << (percore->params.numlev);
    int lastcol = 1 << (percore->params.numlev + (percore->params.log2_nprocs & 1));
    int ilo = (percore->border_n) ? 0 : 1;
    int ihi = (percore->border_s) ? lastrow + 1 : lastrow;
    int jlo = (percore->border_w) ? 0 : 1;
    int jhi = (percore->border_e) ? lastcol + 1 : lastcol;
    int i, j;

    for (i = 1; i <= lastrow; i++) {
        for (j = 1; j <= lastcol; j++) {
            percore->rhs[lev][i][j] = gamma[i][j] * ressqr;
        }
    }

    if (percore->border_n) {
        for (j = jlo; j <= jhi; j++) {
            percore->q[lev][0][j] = gamma[0][j];
        }
    }

    if (percore->border_s) {
        for (j = jlo; j <= jhi; j++) {
            percore->q[lev][lastrow + 1][j] = gamma[lastrow + 1][j];
        }
    }

    if (percore->border_w) {
        for (i = ilo; i <= ihi; i++) {
            percore->q[lev][i][0] = gamma[i][0];
        }
    }

    if (percore->border_e) {
        for (i = ilo; i <= ihi; i++) {
            percore->q[lev][i][lastcol + 1] = gamma[i][lastcol + 1];
        }
    }
}



// one time stuff per core
void prepare(percore_data_t* percore)
{
    int i, j, p;
    double ressqr = percore->params.res * percore->params.res;
    int lastrow = 1 << (percore->params.numlev);
    int lastcol = 1 << (percore->params.numlev + (percore->params.log2_nprocs & 1));
    int ilo = (percore->border_n) ? 0 : 1;
    int ihi = (percore->border_s) ? lastrow + 1 : lastrow;
    int jlo = (percore->border_w) ? 0 : 1;
    int jhi = (percore->border_e) ? lastcol + 1 : lastcol;

    for (i = 0; i < lastrow + 2; i++) {
        for (j = 0; j < lastcol + 2; j++) {
            percore->gamma[0][i][j] = 0.0;
            percore->gamma[1][i][j] = 0.0;
            percore->psib[i][j] = 0.0;
        }
    }

    clear_edges(percore->psib, lastrow, lastcol, 1.0,
                percore->border_n, percore->border_s,
                percore->border_w, percore->border_e);

    // compute psib array (one-time computation) and integrate into psibi
    set_last_level(percore, percore->psib, ressqr);


    // Is the exchange of the borders really necessary? Should be all 0.0
    forward_laplacian_edges(percore->psib, lastcol, lastrow, percore);



    double const1 = 1.0 / (4.0 - ressqr * percore->eig2);

    for (i = 1; i <= lastrow; i++) {
        for (j = 1; j <= lastcol; j++) {
            percore->q[percore->params.numlev - 1][i][j]
                = (percore->psib[i + 1][j] + percore->psib[i - 1][j]
                   + percore->psib[i][j + 1] + percore->psib[i][j - 1] -
                   ressqr * percore->psib[i][j]) * const1;
        }
    }

    multigrid_solver(percore);

    for (i = ilo; i <= ihi; i++) {
        for (j = jlo; j <= jhi; j++) {
            percore->psib[i][j] = percore->q[percore->params.numlev - 1][i][j];
        }
    }



    // calculate psibi individually on every core, then add the local values and
    // read the sum over all
    double local_psibi = 0.0;

    if (percore->border_n) {
        if (percore->border_w) {
            local_psibi = local_psibi + 0.25 * (percore->psib[0][0]);
        }

        if (percore->border_e) {
            local_psibi = local_psibi + 0.25 * (percore->psib[0][lastcol + 1]);
        }

        for (j = 1; j <= lastcol; j++) {
            local_psibi = local_psibi + 0.5 * percore->psib[0][j];
        }
    }

    if (percore->border_s) {
        if (percore->border_w) {
            local_psibi = local_psibi + 0.25 * (percore->psib[lastrow + 1][0]);
        }

        if (percore->border_e) {
            local_psibi = local_psibi + 0.25 * (percore->psib[lastrow + 1][lastcol + 1]);
        }

        for (j = 1; j <= lastcol; j++) {
            local_psibi = local_psibi + 0.5 * percore->psib[lastrow + 1][j];
        }
    }

    if (percore->border_w) {
        for (j = 1; j <= lastrow; j++) {
            local_psibi = local_psibi + 0.5 * percore->psib[j][0];
        }
    }

    if (percore->border_e) {
        for (j = 1; j <= lastrow; j++) {
            local_psibi = local_psibi + 0.5 * percore->psib[j][lastcol + 1];
        }
    }

    for (i = 1; i <= lastrow; i++) {
        for (j = 1; j <= lastcol; j++) {
            local_psibi += percore->psib[i][j];
        }
    }

//     percore->psibi = parop_allreduce_double(0, PAROP_OP_ADD, local_psibi);
    MPI_Allreduce(&local_psibi, &(percore->psibi), 1, MPI_DOUBLE, MPI_SUM, (&percore->c_all));

    // compute input curl of wind stress
    int j_off = percore->colnum * lastcol;
    double const2 = M_PI / (((double)((1 << percore->params.log2_grid) + 1)) * 0.5 * percore->params.res);
    double const3 = -percore->params.t0 * const2;
    double const4 = percore->params.res * const2;

    for (i = ilo; i <= ihi; i++) {
        for (j = jlo; j <= jhi; j++) {

            // compute input curl of wind stress
            percore->tauz[i][j] = const3 * sin((double)(j + j_off) * const4);

            // init psim
            for (p = 0; p < 2; p++) {
                percore->psim[p][i][j] = 0.0;
                percore->psi[p][i][j] = 0.0;
            }
        }
    }
}



void iteration(percore_data_t* percore)
{
    int i, j, p;
    int lastrow = 1 << (percore->params.numlev);
    int lastcol = 1 << (percore->params.numlev + (percore->params.log2_nprocs & 1));
    int ilo = (percore->border_n) ? 0 : 1;
    int ihi = (percore->border_s) ? lastrow + 1 : lastrow;
    int jlo = (percore->border_w) ? 0 : 1;
    int jhi = (percore->border_e) ? lastcol + 1 : lastcol;
    int i_off = percore->rownum * lastrow;
    int j_off = percore->colnum * lastcol;
    double ressqr = percore->params.res * percore->params.res;
    double h30 = percore->params.h3 / percore->params.h0;
    double h10 = percore->params.h1 / percore->params.h0;


    // W1_1 = laplacian(Psi_1)
    // W1_3 = laplacian(Psi_3)
    for (p = 0; p < 2; p++) {
        forward_laplacian_edges(percore->psi[p], lastcol, lastrow, percore);



        laplacian(percore->work1[p], percore->psi[p],
                  1.0 / ressqr, lastrow, lastcol);
        clear_edges(percore->work1[p], lastrow, lastcol, 0.0,
                    percore->border_n, percore->border_s,
                    percore->border_w, percore->border_e);
    }



    for (i = ilo; i <= ihi; i++) {
        for (j = jlo; j <= jhi; j++) {
            double psi1 = percore->psi[0][i][j];
            double psi3 = percore->psi[1][i][j];

            // W2 = Psi_1 - Psi_3
            percore->work2[i][j] = psi1 + psi3;

            // W3 = (h3/h0)*W3 + (h1/h0)*Psi_3
            percore->ga[i][j] = h30 * percore->ga[i][j] + h10 * psi3;
        }
    }

    // W5_1 = Psi_1M
    // W5_3 = Psi_3M
    for (p = 0; p < 2; p++) {
        for (i = ilo; i <= ihi; i++) {
            for (j = jlo; j <= jhi; j++) {
                percore->work5[p][i][j] = percore->psim[p][i][j];
            }
        }
    }



    // W7_1 = laplacian(W5_1)
    // W7_3 = laplacian(W5_3)
    for (p = 0; p < 2; p++) {
        forward_laplacian_edges(percore->work5[p], lastcol, lastrow, percore);
        laplacian(percore->work7[p], percore->work5[p],
                  1.0 / ressqr, lastrow, lastcol);
        clear_edges(percore->work7[p], lastrow, lastcol, 0.0,
                    percore->border_n, percore->border_s,
                    percore->border_w, percore->border_e);
    }

    // Add f values to columns of W1_1 and W1_3
    //
    // Possible bug:
    // In the western and the eastern column, the f index depends on the
    // vertical position (i_off). In all other cases it depends on the
    // horizontal position (j_off)
    for (p = 0; p < 2; p++) {
        for (i = ilo; i <= ihi; i++) {
            for (j = jlo; j <= jhi; j++) {
                percore->work1[p][i][j] += percore->f[j + j_off];
            }
        }

        // correction: subtract previously added f and add different f
        // Is this a bug?
        if (percore->border_w) {
            for (j = 1; j <= lastrow; j++) {
                percore->work1[p][j][0] +=
                    percore->f[j + i_off] - percore->f[0 + j_off];
            }
        }

        if (percore->border_e) {
            for (j = 1; j <= lastrow; j++) {
                percore->work1[p][j][lastcol + 1] +=
                    percore->f[j + i_off] - percore->f[lastcol + 1 + j_off];
            }
        }
    }

    // W5_1 = jacobian(W1_1, Psi_1)
    // W5_3 = jacobian(W1_3, Psi_3)
    for (p = 0; p < 2; p++) {
        forward_edges_and_corners(percore->work1[p], lastcol, lastrow, percore);
        forward_edges_and_corners(percore->psi[p], lastcol, lastrow, percore);
        jacobian(percore->work5[p], percore->work1[p], percore->psi[p],
                 -1. / (12.0 * ressqr), lastrow, lastcol);
        clear_edges(percore->work5[p], lastrow, lastcol, 0.0,
                    percore->border_n, percore->border_s,
                    percore->border_w, percore->border_e);
    }

    // W4_1 = laplacian(W7_1)
    // W4_3 = laplacian(W7_3)
    for (p = 0; p < 2; p++) {
        forward_laplacian_edges(percore->work7[p], lastcol, lastrow, percore);
        laplacian(percore->work1[p], percore->work7[p],
                  1.0 / ressqr, lastrow, lastcol);
        clear_edges(percore->work1[p], lastrow, lastcol, 0.0,
                    percore->border_n, percore->border_s,
                    percore->border_w, percore->border_e);
    }

    // W6 = jacobian(W2, W3)
    forward_edges_and_corners(percore->work2, lastcol, lastrow, percore);
    forward_edges_and_corners(percore->ga, lastcol, lastrow, percore);
    jacobian(percore->gb, percore->work2, percore->ga,
             -1. / (12.*ressqr), lastrow, lastcol);
    clear_edges(percore->gb, lastrow, lastcol, 0.0,
                percore->border_n, percore->border_s,
                percore->border_w, percore->border_e);

    // W2_1 = laplacian(W1_1)
    // W2_3 = laplacian(W1_3)
    for (p = 0; p < 2; p++) {
        forward_laplacian_edges(percore->work1[p], lastcol, lastrow, percore);
        laplacian(percore->work7[p], percore->work1[p],
                  1.0 / ressqr, lastrow, lastcol);
        clear_edges(percore->work7[p], lastrow, lastcol, 0.0,
                    percore->border_n, percore->border_s,
                    percore->border_w, percore->border_e);
    }

    // gamma_a = W5_1 - W5_3 + W6*eig2 + tauz/h1 + W7_1*lf - W7_3*lf
    // gamma_b = W5_1*h1/h0 - W5_3*h3/h0 + tauz/h0 + W7_1*lf*h1/h0 - W7_3*lf*h3/h0
    double h0inv = 1.0 / percore->params.h0;
    double h1inv = 1.0 / percore->params.h1;

    for (i = ilo; i <= ihi; i++) {
        for (j = jlo; j <= jhi; j++) {
            percore->ga[i][j] =
                percore->work5[0][i][j] -
                percore->work5[1][i][j] +
                percore->gb[i][j] * percore->eig2 +
                percore->tauz[i][j] * h1inv +
                percore->work7[0][i][j] * percore->params.lf -
                percore->work7[1][i][j] * percore->params.lf;
            percore->gb[i][j] =
                percore->work5[0][i][j] * h10 +
                percore->work5[1][i][j] * h30 +
                percore->tauz[i][j] * h0inv +
                percore->work7[0][i][j] * h10 * percore->params.lf +
                percore->work7[1][i][j] * h30 * percore->params.lf;
        }
    }

    // Multigrid interpolation of gamma_a
    set_last_level(percore, percore->ga, ressqr);

    for (i = 1; i <= lastrow; i++) {
        for (j = 1; j <= lastcol; j++) {
            percore->q[percore->params.numlev - 1][i][j] = percore->gamma[0][i][j];
        }
    }

    multigrid_solver(percore);

    // Sum up all entries of gamma_a. First locally, than synchronise.
    double local_psiai = 0.0;

    for (i = ilo; i <= ihi; i++) {
        for (j = jlo; j <= jhi; j++) {
            percore->gamma[0][i][j] = percore->q[percore->params.numlev - 1][i][j];
            local_psiai += percore->gamma[0][i][j];
        }
    }

//     local_psiai = parop_allreduce_double(0, PAROP_OP_ADD, local_psiai);
    MPI_Allreduce(&local_psiai, &local_psiai, 1, MPI_DOUBLE, MPI_SUM, (&percore->c_all));


    // Multigrid interpolation of gamma_b
    set_last_level(percore, percore->gb, ressqr);

    for (i = 1; i <= lastrow; i++) {
        for (j = 1; j <= lastcol; j++) {
            percore->q[percore->params.numlev - 1][i][j] = percore->gamma[1][i][j];
        }
    }

    multigrid_solver(percore);

    for (i = ilo; i <= ihi; i++) {
        for (j = jlo; j <= jhi; j++) {
            percore->gamma[1][i][j] = percore->q[percore->params.numlev - 1][i][j];
        }
    }

    for (i = ilo; i <= ihi; i++) {
        for (j = jlo; j <= jhi; j++) {
            double psi0 = percore->psi[0][i][j];
            double psi1 = percore->psi[1][i][j];
            double gamma_a = percore->gamma[0][i][j]
                             - local_psiai / percore->psibi * percore->psib[i][j];
            // augment ga(i,j) with -(psiai/psibi)*psib(i,j)
            double w3 = percore->gamma[1][i][j] + h30 * gamma_a;

            percore->ga[i][j] = w3;
            percore->psi[0][i][j] =
                percore->psim[0][i][j] + 2 * percore->params.dtau * w3;
            percore->psi[1][i][j] =
                percore->psim[1][i][j] + 2 * percore->params.dtau *
                (percore->gamma[1][i][j] - h10 * gamma_a);
            percore->psim[0][i][j] = psi0;
            percore->psim[1][i][j] = psi1;
        }
    }
}







// -----------------------------------------------------------------------------
// Initialisation
// -----------------------------------------------------------------------------


// allocate memory and link it up
double** malloc_linkup(int handle, int x_part, int y_part, void** base_pointer)
{
    double** row_ptr = *base_pointer;//(double **)parop_def_memblock(handle,
    //x_part*y_part*sizeof(double) + y_part*sizeof(double *));
    *base_pointer += (x_part * y_part * sizeof(double) + y_part * sizeof(double*));

    double* ptr = (double*)(row_ptr + y_part);
    int i;

    for (i = 0; i < y_part; i++) {
        row_ptr[i] = ptr;
        ptr += x_part;
    }

    return row_ptr;
}


void init(percore_data_t* percore)
{
    percore->c_all = MPI_New_Comm(0, 4, 4, MPI_COMM_WORLD);
    int i;
    int rank = percore->c_all.rank;
    params_data_t* params = &percore->params;
    int grid_width = (1 << params->log2_grid) + 2;
    int numlev = params->numlev;
    void* base_pointer = (void*)0x20000;
    // precalculations, remain constant for the rest of the program
    // and are identical on every core
    percore->eig2 = -params->h0 * params->f0 * params->f0 / (params->h1 * params->h3 * params->gpr);
    percore->lev_tol = base_pointer;
    base_pointer += numlev * sizeof(double);//parop_def_array_double(MB_LEVTOL, numlev);
    percore->lev_tol[numlev - 1] = params->tolerance;

    percore->f = base_pointer;
    base_pointer += grid_width * sizeof(double);//parop_def_array_double(MB_F, grid_width);
    double ysca = ((double)(grid_width - 1)) * 0.5 * params->res ;

    for (i = 0; i < grid_width; i++) {
        percore->f[i] = params->f0 + params->beta * ((((double)i) * params->res) - ysca);
    }

    // calculate the numbers of the neigbours
    int xprocs = (1 << ((params->log2_nprocs) >> 1));
    int nprocs = 1 << params->log2_nprocs;
    percore->rownum = rank / xprocs;
    percore->colnum = rank % xprocs;

    bool border_n = rank < xprocs;
    bool border_s = rank >= (nprocs - xprocs);
    bool border_w = (rank % xprocs) == 0;
    bool border_e = (rank % xprocs) == (xprocs - 1);

    percore->border_n = border_n;
    percore->border_s = border_s;
    percore->border_w = border_w;
    percore->border_e = border_e;

//     parop_def_direction(DIR_N,
//         border_n ? -1 : rank-xprocs,
//         border_s ? -1 : rank+xprocs);
//     parop_def_direction(DIR_W,
//         border_w ? -1 : rank-1,
//         border_e ? -1 : rank+1);
//     parop_def_direction(DIR_NW,
//         (border_n || border_w) ? -1 : rank-xprocs-1,
//         (border_s || border_e) ? -1 : rank+xprocs+1);
//     parop_def_direction(DIR_NE,
//         (border_n || border_e) ? -1 : rank-xprocs+1,
//         (border_s || border_w) ? -1 : rank+xprocs-1);

    // allocate memory for fields
    percore->one_row = base_pointer;
    base_pointer += (1 << (numlev)) * sizeof(double);//parop_def_array_double(MB_ROW,
//         1 << (numlev));
    percore->one_col = base_pointer;
    base_pointer += (1 << (numlev + (percore->params.log2_nprocs & 1))) * sizeof(double); //parop_def_array_double(MB_COL,
//         1 << (numlev+(percore->params.log2_nprocs&1)));

    int x_part = (1 << (params->log2_grid - ((params->log2_nprocs) >> 1))) + 2;
    int y_part = (1 << (numlev)) + 2;


    percore->ga    = malloc_linkup(MB_GA,   x_part, y_part, &base_pointer);
    percore->gb    = malloc_linkup(MB_GB,   x_part, y_part, &base_pointer);
    percore->psib  = malloc_linkup(MB_PSIB, x_part, y_part, &base_pointer);
    percore->work2 = malloc_linkup(MB_W2,   x_part, y_part, &base_pointer);
    percore->tauz  = malloc_linkup(MB_TAUZ, x_part, y_part, &base_pointer);

    percore->psi[0]   = malloc_linkup(MB_PSI + 0,   x_part, y_part, &base_pointer);
    percore->psim[0]  = malloc_linkup(MB_PSIM + 0,  x_part, y_part, &base_pointer);
    percore->work1[0] = malloc_linkup(MB_W1 + 0,    x_part, y_part, &base_pointer);
    percore->work5[0] = malloc_linkup(MB_W5 + 0,    x_part, y_part, &base_pointer);
    percore->work7[0] = malloc_linkup(MB_W7 + 0,    x_part, y_part, &base_pointer);
    percore->gamma[0] = malloc_linkup(MB_GAMMA + 0, x_part, y_part, &base_pointer);

    percore->psi[1]   = malloc_linkup(MB_PSI + 1,   x_part, y_part, &base_pointer);
    percore->psim[1]  = malloc_linkup(MB_PSIM + 1,  x_part, y_part, &base_pointer);
    percore->work1[1] = malloc_linkup(MB_W1 + 1,    x_part, y_part, &base_pointer);
    percore->work5[1] = malloc_linkup(MB_W5 + 1,    x_part, y_part, &base_pointer);
    percore->work7[1] = malloc_linkup(MB_W7 + 1,    x_part, y_part, &base_pointer);
    percore->gamma[1] = malloc_linkup(MB_GAMMA + 1, x_part, y_part, &base_pointer);

    for (i = 0; i < numlev; i++) {
        x_part = (1 << ((percore->params.log2_nprocs & 1) + 1 + i)) + 2;
        y_part = (1 << (1 + i)) + 2;
        percore->q  [i] = malloc_linkup(MB_Q  + i, x_part, y_part, &base_pointer);
        percore->rhs[i] = malloc_linkup(MB_RHS + i, x_part, y_part, &base_pointer);
    }
}



// run this function once per core
void parallel(void* p)
{
    params_data_t* params = p;
    percore_data_t percore;
    percore.params = *params;

    if (fgmp_addr_x(MPI_COMM_WORLD->address) >= 4 || fgmp_addr_y(MPI_COMM_WORLD->address) >= 4) {
        return;
    }

    init(&percore);
    prepare(&percore);

    double t;

    for (t = 0.0; t < params->time_interval; t += params->dtau)
        iteration(&percore);
}



int config(int argc, char* argv[], void* p, int available_threads)
{
    params_data_t* params = (params_data_t*)p;
    int nprocs = 16;//(&percore->c_all)->size;
    int problem_size = DEFAULT_LOG2_N;

//     if (argc != 3) {
//         harness_print_string("Usage: ocean <#threads> <problem size>\n"
//             "  <#threads>     Number of threads. Must be a power of 2.\n"
//             "  <problem size> Simulate a quadratic ocean with an edge length "
//             "of 2**N+2.\n"
//             "Invalid arguments, using default values instead: ocean ");
//
//         // use default thread count if it is not given by the architecture
//         nprocs = (available_threads == 0)
//             ? (1 << DEFAULT_LOG2_P)
//             : available_threads;
//         problem_size = DEFAULT_LOG2_N;
//
//         harness_print_int32(nprocs);
//         harness_print_string(" ");
//         harness_print_int32(problem_size);
//         harness_print_string("\n\n");
//     } else {
//         nprocs = harness_atoi(argv[1]);
//         if (available_threads!=0 && nprocs > available_threads)
//             nprocs = available_threads;
//         problem_size = harness_atoi(argv[2]);
//     }

    // nprocs must be a power of 2
    int log2_nprocs = 0;

    while ((2 << log2_nprocs) <= nprocs) log2_nprocs++;

    params->log2_nprocs = log2_nprocs;

    params->numlev = problem_size - ((log2_nprocs + 1) >> 1);
//     if (params->numlev <= 0)
//         harness_fatal("Must have at least 2 grid points per thread in "
//             "each dimension");
//     if (params->numlev > MAX_NUMLEV)
//         harness_fatal("Too many grid points per thread");
    params->log2_grid = problem_size;

    // initialise constants
    params->h1 = 1000.0;
    params->h3 = 4000.0;
    params->h0 = 5000.0;
    params->lf = -5.12e11;
    params->f0 = 8.3e-5;
    params->beta = 2.0e-11;
    params->gpr = 0.02;
    params->t0 = 0.5e-4 ;

    // initialise parameters
    params->res = 20000.0;
    params->dtau = 28800.0;
    params->tolerance = 1e-7;
    params->time_interval = 2.0 * 86400.0;
    params->maxwork = 10000 * (1 << (2 * (params->numlev - 1)));

//     harness_print_string("Parallel Benchmark OCEAN\n  size: ");
//     harness_print_int32((1 << problem_size)+2);
//     harness_print_string("\n  using ");
//     harness_print_int32(1 << log2_nprocs);
//     harness_print_string(" of ");
//     harness_print_int32(available_threads);
//     harness_print_string(" threads\n");

    return 1 << log2_nprocs;
}



int main(int argc, char** argv)
{
    MPI_Init(0, 0);

    params_data_t params;
    config(argc, argv, &params, 0);
    parallel(&params);

    return 0;
    /*    return harness_spmd_main(argc, argv,
            sizeof(params_data_t),
            config,
            parallel)*/;
}


