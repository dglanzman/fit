#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

#define BLOCK_SIZE 128

struct wsv {
    int w;
    int h;
    double * data;
};

void read_wsv(struct wsv * out, FILE * infile) {
    
    // copy input file stream to buffer and get its dimensions
    char buf[BLOCK_SIZE];
    FILE * buffer = tmpfile();
    int size = 0;
    size_t num_read;
    do {
        num_read = fread(buf, 1, BLOCK_SIZE, infile);
        fwrite(buf, 1, num_read, buffer);
        size += num_read;
    } while (num_read == BLOCK_SIZE);
    rewind(buffer);
    long start = ftell(buffer);
    fscanf(buffer, "%*lf ");
    long one = ftell(buffer);
    rewind(buffer);
    while(fgetc(buffer) != '\n');
    long line = ftell(buffer);
    fseek(buffer, 0, SEEK_END);
    long end = ftell(buffer);
    rewind(buffer);
    out->w = (line - start) / (one - start);
    out->h = (end - start) / (line - start);

    // copy data into array buffer
    size = out->w * out->h;
    out->data = malloc(size * sizeof (double));
    for (int i = 0; i < size; i++) {
        fscanf(buffer, "%lf", &(out->data[i]));
    }
    fclose(buffer);
}

void get_data_stdin(gsl_vector ** x, gsl_vector ** y) {
    struct wsv raw_data;
    read_wsv(&raw_data, stdin);
    *x = gsl_vector_alloc(raw_data.h);
    *y = gsl_vector_alloc(raw_data.h);
    for (int i = 0; i < raw_data.h; i++) {
        gsl_vector_set(*x, i, raw_data.data[i * raw_data.w]);
        gsl_vector_set(*y, i, raw_data.data[i * raw_data.w + 1]);
    }
    free(raw_data.data);
}

void least_squares(gsl_vector * x, gsl_vector * y, gsl_vector * c) {

    int order = c->size; 

    // generate polynomial model
    gsl_matrix * model = gsl_matrix_alloc(x->size, order + 1);
    gsl_vector * powers = gsl_vector_alloc(x->size);
    gsl_vector_set_all(powers, 1);
    for (int i = 0; i <= order; i++) {
        gsl_matrix_set_col(model, i, powers);
        gsl_vector_mul(powers, x);
    }

    // least-squares method is X'y = X'Xc
    // calculate b = X'y
    gsl_vector * b = gsl_vector_alloc(x->size);
    gsl_blas_dgemv(CblasTrans, 1, model, y, 0, b);

    // calculate A = X'X
    gsl_matrix * A = gsl_matrix_alloc(x->size, x->size);
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, model, model, 0, A);
 
    // we now solve the Ac = b system for the coefficients c
    gsl_permutation * p = gsl_permutation_alloc(x->size);
    int sign;
    gsl_linalg_LU_decomp(A, p, &sign);
    gsl_linalg_LU_solve(A, p, b, c);


    /* free memory
    gsl_matrix_free(model);
    gsl_vector_free(powers);
    gsl_vector_free(b);
    gsl_matrix_free(A);
    gsl_permutation_free(p); */
}

int main() {

    // read in x and y column
    gsl_vector * x, * y;
    int order;
    scanf("order=%d\n", &order);
    get_data_stdin(&x, &y);

    // find least-squares coefficients
    gsl_vector * c = gsl_vector_alloc(order + 1);
    least_squares(x, y, c);

    gsl_vector_fprintf(stdout, c, "%f");
    printf("\n");

    gsl_vector_free(x);
    gsl_vector_free(y);
    gsl_vector_free(c);
}

