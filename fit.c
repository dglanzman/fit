#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

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
}

int main() {
    gsl_vector * x, * y;
    int order;
    scanf("order=%d\n", &order);
    get_data_stdin(&x, &y);
    gsl_vector_fprintf(stdout, x, "[%f]");
    printf("\n");
    gsl_vector_fprintf(stdout, y, "[%f]");
    printf("\n");
    gsl_matrix * A = gsl_matrix_alloc(x->size, order + 1);
    gsl_vector * powers = gsl_vector_alloc(x->size);
    gsl_vector_set_all(powers, 1);
    for (int i = 0; i <= order; i++) {
        gsl_vector_view col = gsl_matrix_column(A, i);
        gsl_vector_memcpy(&(col.vector), powers);
        gsl_vector_mul(powers, x);
    }
    gsl_matrix_fprintf(stdout, A, "[%f]");
}
