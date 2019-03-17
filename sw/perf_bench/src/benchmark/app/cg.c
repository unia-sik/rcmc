// WCET-aware clone of CG from the NAS Parallel Bechmark 3.3 suite.
// Completely rewritten in C.
//
// The original implementation is written in FORTRAN and is available at
// http://www.nas.nasa.gov/Software/NPB/
//
// Difference to original:
//   * uses allreduce instead of hand-written logarithmic message exchange
//   * no longer do iteration 1 twice to warm caches and page tables
//   * n must be a multiple of the threads per column


#include "mpi.h"
#include "stdbool.h"
#include <math.h>

#define WCET_ANALYSIS
// default number of threads
#ifndef DEFAULT_LOG2_P
#define DEFAULT_LOG2_P 3
#endif

#ifndef DEFAULT_CLASS
#define DEFAULT_CLASS 1
#endif


#define CG_ITERATIONS   15


#define A_COLIDX        0
#define A_ROWSTR        1
#define A_A             2
#define A_X             3
#define A_Z             4
#define A_P             5
#define A_Q             6
#define A_R             7
#define A_W             8

#define D_TRANSPOSE     0

#define C_ALL           0       // predefined
#define C_ROW           1


// parameters are computed at the program start, but remain constant
// during the parallel execution
typedef struct params_data_s {
    int log2_num_procs;

    // configuration
    int n;                      // matrix size
    int nonzero_per_row;
    int iterations;
    double eigenvalue_shift;
    double zeta_verify;

} params_data_t;


typedef struct percore_data_s {
    // 1. individual per core, but fixed (only depending on core id)

    // 2. pre-calculated tables, once written at beginning of each thread
    //    identical on every core

    // 3. only locally used (by owning core)
    int32_t* colidx;
    int32_t* rowstr;
    double* a;
    double* x;
    double* z;
    double* p;
    double* q;
    double* r;
    double* w;


    int exch_proc;
    mpi_communicator_t c_all;
    mpi_communicator_t c_row;

    // 4. results
} percore_data_t;






uint64_t randNASseed = 314159265;

uint64_t randNAS(uint64_t* seed)
{
    return *seed = ((*seed * 1220703125) & 0x00003fffffffffff);
}

double randNAS_double()
{
    return 0x1p-46 * randNAS(&randNASseed);
}







void sprnvc(int n, int nz, double* v, int* iv, int* nzloc, bool* mark)
{
    int i, ii;
    int nzv = 0;
    int nzrow = 0;

    //  nn1 is the smallest power of two not less than n
    int nn1 = 2;

    while (nn1 < n) nn1 = 2 * nn1;

    while (nzv < nz) {
        double vecelt = randNAS_double();

        // generate evenly distributed random number between 1 and n
        i = randNAS_double() * nn1 + 1;

        // entry already occupied?
        if (i <= n && mark[i] == 0) {
            mark[i] = 1;
            nzloc[++nzrow] = i;
            v[++nzv] = vecelt;
            iv[nzv] = i;
        }
    }

    // clean mark for next use
    for (ii = 1; ii <= nzrow; ii++) {
        i = nzloc[ii];
        mark[i] = 0;
    }
}


//  set the ith element of the sparse vector (v, iv)
int vecset(double* v, int* iv, int nzv, int i, double val)
{
    int k;

    for (k = 1; k <= nzv; k++) {
        if (iv[k] == i) {
            v[k] = val;
            return nzv;
        }
    }

    nzv++;
    v[nzv] = val;
    iv[nzv] = i;
    return nzv;
}


// generate a sparse matrix
void sparse(double a[], int colidx[], int rowstr[],
            int n, int arow[], int acol[], double aelt[],
            int firstrow, int lastrow, double x[],
            int nzloc[], bool mark[], int nnza)
{
    int nrows = lastrow - firstrow + 1;
    int i, j, jajp1, nza, k, nzrow;
    double xi;

    for (j = 1; j <= n; ++j) {
        rowstr[j] = 0;
        mark[j] = false;
    }

    rowstr[n + 1] = 0;

    for (nza = 1; nza <= nnza; ++nza) {
        j = (arow[nza] - firstrow + 1) + 1;
        rowstr[j] = rowstr[j] + 1;
    }

    rowstr[1] = 1;

    for (j = 2; j <= nrows + 1; ++j) {
        rowstr[j] = rowstr[j] + rowstr[j - 1];
    }

    for (nza = 1; nza <= nnza; ++nza) {
        j = arow[nza] - firstrow + 1;
        k = rowstr[j];
        a[k] = aelt[nza];
        colidx[k] = acol[nza];
        rowstr[j] = rowstr[j] + 1;
    }

    for (j = nrows; j >= 1; --j) {
        rowstr[j + 1] = rowstr[j];
    }

    rowstr[1] = 1;

    nza = 0;

    for (i = 1; i <= n; ++i) {
        x[i] = 0.0;
        mark[i] = false;
    }

    jajp1 = rowstr[1];

    for (j = 1; j <= nrows; ++j) {
        nzrow = 0;

        for (k = jajp1; k <= rowstr[j + 1] - 1; ++k) {
            i = colidx[k];
            x[i] = x[i] + a[k];

            if ((!mark[i]) && (x[i] != 0.0)) {
                mark[i] = true;
                nzrow = nzrow + 1;
                nzloc[nzrow] = i;
            }
        }

        for (k = 1; k <= nzrow; ++k) {
            i = nzloc[k];
            mark[i] = false;
            xi = x[i];
            x[i] = 0.0;

            if (xi != 0.0) {
                nza = nza + 1;
                a[nza] = xi;
                colidx[nza] = i;
            }
        }

        jajp1 = rowstr[j + 1];
        rowstr[j + 1] = nza + rowstr[1];
    }

    //harness_print_string("number of nonzeros = ");
    //harness_print_int32(nza);
    //harness_print_string("\n");
}


void makea(
    int n,
    int nz,
    double a[],
    int colidx[],
    int rowstr[],
    int nonzer,
    int firstrow,
    int lastrow,
    int firstcol,
    int lastcol,
    double rcond,
    double shift,
    void* base_pointer
)
{
// #ifdef WCET_ANALYSIS
//     // CAUTION: a lot of memory  from the stack (ca 30*n bytes)
//     double aelt[nz+1];
//     double v[n+1+1];
//     int32_t arow[nz+1];
//     int32_t acol[nz+1];
//     int32_t nzloc[n+1];
//     bool mark[n+1];
// #else
    double* aelt   = base_pointer;
    base_pointer += sizeof(double) * (nz + 1);//machine_malloc(sizeof(double)*(nz+1));
    double* v      = base_pointer;
    base_pointer += sizeof(double) * (n + 1 + 1);//machine_malloc(sizeof(double)*(n+1+1));
    int32_t* arow  = base_pointer;
    base_pointer += sizeof(int32_t) * (nz + 1);//machine_malloc(sizeof(int32_t)*(nz+1));
    int32_t* acol  = base_pointer;
    base_pointer += sizeof(int32_t) * (nz + 1);//machine_malloc(sizeof(int32_t)*(nz+1));
    int32_t* nzloc = base_pointer;
    base_pointer += sizeof(int32_t) * (n + 1);//machine_malloc(sizeof(int32_t)*(n+1));
    bool*    mark  = base_pointer;
    base_pointer += sizeof(bool) * (n + 1);//machine_malloc(sizeof(bool)*(n+1));
// #endif

    // nonzer is approximately  sqrt(nnza/n)
    int i, ivelt, ivelt1;
    double size = 1.0;
    double ratio = pow(rcond, 1.0 / (double) n);
    int nnza = 0;

    for (i = 1; i <= n; i++) mark[i] = 0;

    for (i = 1; i <= n; i++) {
        sprnvc(n, nonzer, v, colidx, nzloc, mark);
        int nzv = vecset(v, colidx, nonzer, i, 0.5);

        for (ivelt = 1; ivelt <= nzv; ++ivelt) {
            int jcol = colidx[ivelt];

            if (jcol >= firstcol && jcol <= lastcol) {
                double scale = size * v[ivelt];

                for (ivelt1 = 1; ivelt1 <= nzv; ++ivelt1) {
                    int irow = colidx[ivelt1];

                    if (irow >= firstrow && irow <= lastrow) {
                        nnza = nnza + 1;
//                         if (nnza > nz)
//                             harness_fatal(
//                                 "Space for matrix elements exceeded in makea" );
                        acol[nnza] = jcol;
                        arow[nnza] = irow;
                        aelt[nnza] = v[ivelt1] * scale;
                    }
                }
            }
        }

        size = size * ratio;
    }

    for (i = firstrow; i <= lastrow; ++i) {
        if (i >= firstcol && i <= lastcol) {
            nnza = nnza + 1;

            if (nnza > nz) break;

            acol[nnza] = i;
            arow[nnza] = i;
            aelt[nnza] = rcond - shift;
        }
    }

    sparse(a, colidx, rowstr, n, arow, acol, aelt, firstrow, lastrow, v, nzloc,
           mark, nnza);

// #ifndef WCET_ANALYSIS
//     machine_free(aelt);
//     machine_free(v);
//     machine_free(arow);
//     machine_free(acol);
//     machine_free(nzloc);
//     machine_free(mark);
// #endif

}

void init(percore_data_t* percore, params_data_t* params)
{
    percore->c_all = MPI_New_Comm(0, 4, 4, MPI_COMM_WORLD);




    int my_rank = percore->c_all.rank;
    int n = params->n;

    int prows    = 1 << (params->log2_num_procs >> 1);
    int pcols    = 1 << ((params->log2_num_procs + 1) >> 1);
    int nrows    = n / prows;
    int ncols    = n / pcols;

    // If n is evenly divisible by pcols it is also evenly divisible by prows
//     machine_assert( (n%pcols)==0 ); // ignore special cases


    if (pcols == prows) {
        percore->exch_proc = (my_rank % prows) * prows + my_rank / prows;
    } else {
//         machine_assert(pcols==2*prows && nrows==2*ncols);
        percore->exch_proc = 2 * (((my_rank / 2) % prows) * prows + my_rank / 2 / prows)
                             + my_rank % 2;
    }

//     parop_def_direction(D_TRANSPOSE, exch_proc, exch_proc);

    int i;
//     int color[&(percore->c_all)->size];
//     for (i=0; i<&(percore->c_all)->size; i++) color[i] = i/pcols;

//     parop_def_collection(C_ROW, color);

    percore->c_row = MPI_New_Comm(
                         fgmp_addr_to_rank(fgmp_addr_gen(0, fgmp_addr_y((percore->c_all).address)), &(percore->c_all)),
                         (percore->c_all).width,
                         1,
                         &(percore->c_all)
                     );


    randNAS_double(); // only needed for compatibility to FORTRAN version

    int nonzer = params->nonzero_per_row;
    int shift = params->eigenvalue_shift;
    int num_procs = 1 << params->log2_num_procs;
    int nz = n * (nonzer + 1) / num_procs * (nonzer + 1) + nonzer
             + n * (nonzer + 2 + num_procs / 256) / pcols;
    void* base_pointer = (void*)0x20000;
//     percore->colidx = parop_def_array_int32(A_COLIDX, nz+1);
//     percore->rowstr = parop_def_array_int32(A_ROWSTR, n+1+1);
//     percore->a = parop_def_array_double(A_A, nz+1);


    percore->colidx = base_pointer;
    base_pointer += sizeof(int32_t) * (nz + 1); //parop_def_array_int32(A_COLIDX, nz+1);
    percore->rowstr = base_pointer;
    base_pointer += sizeof(int32_t) * (n + 1 + 1);//parop_def_array_int32(A_ROWSTR, n+1+1);
    percore->a = base_pointer;
    base_pointer += sizeof(double) * (nz + 1);//parop_def_array_double(A_A, nz+1);

    int proc_row = my_rank / pcols;
    int proc_col = my_rank % pcols;
    int firstrow = proc_row * nrows + 1;
    int firstcol = proc_col * ncols + 1;
    int lastrow  = firstrow - 1 + nrows;
    int lastcol  = firstcol - 1 + ncols;
    // Set up partition's sparse random matrix for given class size
    // Uses a lot of temporary memory, therefore allocate memory after
    // calling it, if possible.


    makea(n, nz, percore->a, percore->colidx, percore->rowstr, nonzer,
          firstrow, lastrow, firstcol, lastcol, 0.1, shift, base_pointer);

    int j, k;

    for (j = 1; j <= nrows; j++)
        for (k = percore->rowstr[j]; k <= percore->rowstr[j + 1] - 1; k++)
            percore->colidx[k] -= firstcol - 1;

    percore->x =  base_pointer;
    base_pointer += sizeof(double) * (nrows + 2 + 1); //parop_def_array_double(A_X, nrows+2+1);
    percore->z =  base_pointer;
    base_pointer += sizeof(double) * (nrows + 2 + 1); //parop_def_array_double(A_Z, nrows+2+1);
    percore->p =  base_pointer;
    base_pointer += sizeof(double) * (nrows + 2 + 1); //parop_def_array_double(A_P, nrows+2+1);
    percore->q =  base_pointer;
    base_pointer += sizeof(double) * (nrows + 2 + 1); //parop_def_array_double(A_Q, nrows+2+1);
    percore->r =  base_pointer;
    base_pointer += sizeof(double) * (nrows + 2 + 1); //parop_def_array_double(A_R, nrows+2+1);
    percore->w =  base_pointer;
    base_pointer += sizeof(double) * (nrows + 2 + 1); //parop_def_array_double(A_W, nrows+2+1);

    for (j = 1; j <= nrows + 1; j++) percore->x[j] = 1.0;
}



double conjugent_gradient(percore_data_t* percore, int n, int log2_num_procs)
{
    int j, k;
    double d, sum, rho;

    int my_rank = (percore->c_all).rank;

    int* colidx  = percore->colidx;
    int* rowstr  = percore->rowstr;
    double* a    = percore->a; // input
    double* x    = percore->x; // input
    double* z    = percore->z; // output
    double* p    = percore->p;
    double* q    = percore->q;
    double* r    = percore->r;
    double* w    = percore->w; // temp

    int prows    = 1 << (log2_num_procs >> 1);
    int pcols    = 1 << ((log2_num_procs + 1) >> 1);
    int nrows    = n / prows;
    int ncols    = n / pcols;

    int upper_half = (pcols != prows) && ((my_rank & 1) != 0) ? ncols : 0;
    // If there are more cols than rows, only half of q must be
    // sent to the transpose node. This offset indicates, if the upper or
    // the lower half must be sent.



    for (j = 1; j <= nrows + 1; j++) {
        z[j] = 0.0;
        r[j] = x[j];
        p[j] = r[j];
    }

    // rho = r.r
    sum = 0.0;

    for (j = 1; j <= ncols; j++) sum = sum + r[j] * r[j];

//     rho = parop_allreduce_double(C_ROW, PAROP_OP_ADD, sum);

    MPI_Allreduce(&sum, &rho, 1, MPI_DOUBLE, MPI_SUM, &(percore->c_row));

    // main conj grad iteration loop
    int cgit;

    for (cgit = 0; cgit < CG_ITERATIONS; cgit++) {

        // q = A.p
        // The partition submatrix-vector multiply: use workspace w
        for (j = 1; j <= nrows; j++) {
            sum = 0.0;

            for (k = rowstr[j]; k <= rowstr[j + 1] - 1; k++)
                sum = sum + a[k] * p[colidx[k]];

            w[j] = sum;
        }

        // Sum the partition submatrix-vec A.p's across rows
        // IMPROVE: throws away half of q, if pcols!=prows
//         parop_allreduce_multi_double(C_ROW, PAROP_OP_ADD, , nrows, &w[1]);
        MPI_Allreduce(&w[1], &q[1], nrows, MPI_DOUBLE, MPI_SUM, &(percore->c_row));

        // Exchange piece of q with transpose processor:
        if (pcols != 1) {
            for (j = 1; j <= ncols; j++) w[j] = q[j + upper_half];

//             parop_forward_begin();
//             parop_forward_double(D_TRANSPOSE, ncols, A_Q, 1, 1, A_W, 1, 1);
            if (percore->c_all.rank != percore->exch_proc) {
                MPI_Status status;
                MPI_Sendrecv((percore->w + 1), ncols, MPI_DOUBLE, percore->exch_proc, 0, (percore->q + 1), ncols, MPI_DOUBLE, percore->exch_proc, 0, &(percore->c_all), &status);
            } else {
                for (int ii = 0; ii < ncols; ii++) {
                    double tmp = (percore->w + 1)[ii];
                    (percore->w + 1)[ii] = (percore->q + 1)[ii];
                    (percore->q + 1)[ii] = tmp;
                }
            }

//             parop_forward_end();
        }

        // d = p.q
        sum = 0.0;

        for (j = 1; j <= ncols; j++) sum = sum + p[j] * q[j];

//         d = parop_allreduce_double(C_ROW, PAROP_OP_ADD, sum);
        MPI_Allreduce(&sum, &d, 1, MPI_DOUBLE, MPI_SUM, &(percore->c_row));


        // alpha = rho / (p.q)
        // z = z + alpha*p
        // r = r - alpha*q
        double alpha = rho / d;

        for (j = 1; j <= ncols; j++) {
            z[j] = z[j] + alpha * p[j];
            r[j] = r[j] - alpha * q[j];
        }

        // rho = r.r
        double rho_new;
        sum = 0.0;

        for (j = 1; j <= ncols; j++) sum = sum + r[j] * r[j];

//         rho_new = parop_allreduce_double(C_ROW, PAROP_OP_ADD, sum);
        MPI_Allreduce(&sum, &rho_new, 1, MPI_DOUBLE, MPI_SUM, &(percore->c_row));
        // p = r + beta*p
        double beta = rho_new / rho;

        for (j = 1; j <= ncols; j++) p[j] = r[j] + beta * p[j];

        rho = rho_new;

    }


    // Compute residual norm explicitly:  ||r|| = ||x - A.z||
    // First, form A.z
    // The partition submatrix-vector multiply
    for (j = 1; j <= nrows; j++) {
        sum = 0.0;

        for (k = rowstr[j]; k <= rowstr[j + 1] - 1; k++)
            sum = sum + a[k] * z[colidx[k]];

        w[j] = sum;
    }

    // Sum the partition submatrix-vec A.z's across rows
//     parop_allreduce_multi_double(C_ROW, PAROP_OP_ADD, &r[1], nrows, &w[1]);
    MPI_Allreduce(&w[1], &r[1], nrows, MPI_DOUBLE, MPI_SUM, &(percore->c_row));

    // Exchange piece of q with transpose processor:
    if (pcols != 1) {
        for (j = 1; j <= ncols; j++) w[j] = r[j + upper_half];

//         parop_forward_begin();
//         parop_forward_double(D_TRANSPOSE, ncols, A_R, 1, 1, A_W, 1, 1);
        if (percore->c_all.rank != percore->exch_proc) {
            MPI_Status status;
            MPI_Sendrecv((percore->w + 1), ncols, MPI_DOUBLE, percore->exch_proc, 0, (percore->r + 1), ncols, MPI_DOUBLE, percore->exch_proc, 0, &(percore->c_all), &status);
        } else {
            for (int ii = 0; ii < ncols; ii++) {
                double tmp = (percore->w + 1)[ii];
                (percore->w + 1)[ii] = (percore->r + 1)[ii];
                (percore->r + 1)[ii] = tmp;
            }
        }

//         parop_forward_end();
    }

    // At this point, r contains A.z
    sum = 0.0;

    for (j = 1; j <= ncols; j++) {
        d = x[j] - r[j];
        sum = sum + d * d;
    }

//     d = parop_allreduce_double(C_ROW, PAROP_OP_ADD, sum);
    MPI_Allreduce(&sum, &d, 1, MPI_DOUBLE, MPI_SUM, &(percore->c_row));
    return d;
}



void iteration(percore_data_t* percore, params_data_t* params,
               double* rsquare, double* zeta)
{
    int i;

    int prows = 1 << (params->log2_num_procs >> 1);
    int nrows = params->n / prows;

    // main iteration for inverse power method
    *rsquare = conjugent_gradient(percore, params->n,
                                  params->log2_num_procs);

    double local[2], total[2];
    local[0] = 0.0;
    local[1] = 0.0;

    for (i = 1; i <= nrows; i++) {
        local[0] += percore->x[i] * percore->z[i];
        local[1] += percore->z[i] * percore->z[i];
    }

//     parop_allreduce_multi_double(C_ALL, PAROP_OP_ADD, total, 2, local);
    MPI_Allreduce(local, total, 2, MPI_DOUBLE, MPI_SUM, &(percore->c_all));
    *zeta = params->eigenvalue_shift + 1.0 / total[0];

    // normalize z to obtain x
    double norm = 1.0 / sqrt(total[1]);

    for (i = 1; i <= nrows; i++)
        percore->x[i] = norm * percore->z[i];

}

static struct {
    int n;
    int nonzero_per_row;
    int iterations;
    int eigenvalue_shift;
    double verify;
} parameters[8] = {
    {    256,  3,  5,    8, 0x1.7CCE0CBDp+1}, // very small
    {   1400,  7,  15,   10, 8.5971775078648}, // class S
    {   7000,  8,  15,   12, 10.362595087124}, // class W
    {  14000, 11,  15,   20, 17.130235054029}, // class A
    {  75000, 13,  75,   60, 22.712745482631}, // class B
    { 150000, 15,  75,  110, 28.973605592845}, // class C
    {1500000, 21, 100,  500, 52.514532105794}, // class D
    {9000000, 26, 100, 1500, 77.522164599383}, // class E
};


int config(int argc, char* argv[], void* p, int available_threads)
{
    params_data_t* params = p;
    int nprocs = 16;//&(percore->c_all)->size;
    int class = DEFAULT_CLASS;

//     if (argc != 3) {
//         harness_print_string("Usage: cg <#threads> <problem size>\n"
//             "  <#threads>     Number of threads. Must be a power of 2.\n"
//             "  <problem size> 1 to 6 corresponds to NPB classes "
//             "S, W, A, B, C D, E\n"
//             "                 0 is very small\n"
//             "Invalid arguments, using default values instead: cg ");
//
//         // use default thread count if it is not given by the architecture
//         nprocs = (available_threads == 0)
//             ? (1 << DEFAULT_LOG2_P)
//             : available_threads;
//         class = DEFAULT_CLASS;
//
//         harness_print_int32(nprocs);
//         harness_print_string(" ");
//         harness_print_int32(class);
//         harness_print_string("\n\n");
//     } else {
//         nprocs = harness_atoi(argv[1]);
//         if (available_threads!=0 && nprocs > available_threads)
//             nprocs = available_threads;
//         class = harness_atoi(argv[2]);
//     }

    // nprocs must be a power of 2
    int log2_nprocs = 0;

    while ((2 << log2_nprocs) <= nprocs) log2_nprocs++;

    params->log2_num_procs = log2_nprocs;

    if (class < 8) {
        params->n                = parameters[class].n;
        params->nonzero_per_row  = parameters[class].nonzero_per_row;
        params->iterations       = parameters[class].iterations;
        params->eigenvalue_shift = parameters[class].eigenvalue_shift;
        params->zeta_verify      = parameters[class].verify;
    } else {
        params->n                = 2048 << (2 * class);
        params->nonzero_per_row  = 2 + 4 * class;
        params->iterations       = 15 + 16 * class;
        params->eigenvalue_shift = 16 << class;
        params->zeta_verify      = 0.0;
    }

    int pcols = 1 << ((log2_nprocs + 1) >> 1);
//     if ((params->n % pcols) != 0)
//         harness_fatal("Matrix size must be a multiple of the threads/column");
//
//     harness_print_string("Parallel Benchmark CG\n  size: ");
//     harness_print_int32(params->n);
//     harness_print_string(" iterations: ");
//     harness_print_int32(params->iterations);
//     harness_print_string(" nonzeros/row: ");
//     harness_print_int32(params->nonzero_per_row);
//     harness_print_string(" eigenvalue shift: ");
//     harness_print_int32(params->eigenvalue_shift);
//     harness_print_string("\n  using ");
//     harness_print_int32(1 << log2_nprocs);
//     harness_print_string(" of ");
//     harness_print_int32(available_threads);
//     harness_print_string(" threads\n");

    return 1 << log2_nprocs;
}


int parallel(void* p)
{
    params_data_t* params = p;
    percore_data_t percore; // thread local storage

    if (fgmp_addr_x(MPI_COMM_WORLD->address) >= 4 || fgmp_addr_y(MPI_COMM_WORLD->address) >= 4) {
        return 0;
    }


    init(&percore, params);

//     parop_barrier(C_ALL);
    MPI_Barrier(&(percore.c_all));



    int i;
    double rsquare = 0.0, zeta = 0.0; // avoid warning

    for (i = 1; i <= params->iterations; i++) {
        iteration(&percore, params, &rsquare, &zeta);
//         PAROP_ONCE {
//             harness_print_string("iter ");
//             harness_print_int32(i);
//             harness_print_string(" r**2=");
//             harness_print_double(rsquare);
//             harness_print_string(" zeta=");
//             harness_print_double(zeta);
//             harness_print_string("\n");
//         }
    }

    // check if zeta has the correct value
//     PAROP_ONCE {
    double error = (zeta - params->zeta_verify) / params->zeta_verify;

    if (error < 0.0) error = -error; // avoid fabs

//         harness_print_string("error=");
//         harness_print_double(error);
//         harness_print_string("\nverification ");
    if (error <= 1e-10)
//             harness_print_string("successful\n");
        return 0;
    else
        return -1;

//             harness_print_string("FAILED\n");
//     }
}


int main(int argc, char** argv)
{
    MPI_Init(0, 0);

    params_data_t params;
    config(argc, argv, &params, 0);
    return parallel(&params);
//     return harness_spmd_main(argc, argv,
//         sizeof(params_data_t),
//         config,
//         parallel);
}



