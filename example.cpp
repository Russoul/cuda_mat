/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#include <typeinfo> // for usage of C++ typeid
#include <cuda_runtime.h>
#include <vector>
#include <conio.h>
#include <sstream>
#include <iostream>

#include "cublas_v2.h"
#include "cusparse_v2.h"
#include "helper_cusolver.h"
#include "mmio.h"

#include "mmio_wrapper.h"

#include "helper_cuda.h"

#include "pbicgstab.h"




void test_A0_d(){
	int n;
	int m;
	int nnz;
	double *A0;
	int *iA0;
	int *jA0;
    loadMMSparseMatrix(const_cast<char *>("mat3_A0.mtx"), 'd', true, &m, &n, &nnz, &A0, &iA0, &jA0);

    int nnz1;

    double *d;
    int *id;
    int *jd;

    loadMMSparseMatrix(const_cast<char *>("vec3_d.mtx"), 'd', true, &m, &n, &nnz1, &d, &id, &jd);

	int nnz2;

	double *b;
	int *ib;
	int *jb;

	loadMMSparseMatrix(const_cast<char *>("vec3.mtx"), 'd', true, &m, &n, &nnz2, &b, &ib, &jb);

    double d_dense[3];
	double b_dense[3];

    double x0[3] = {1,1,1};

    for (int i = 0; i < nnz1; ++i) {
        std::cout << d[i] << std::endl;

    }

    std::cout << "--" << std::endl;

    for (int i = 0; i < m + 1; ++i) {
        std::cout << id[i] << std::endl;

    }


    toDenseVector(m, nnz1, d, id, d_dense);
	toDenseVector(m, nnz2, b, ib, b_dense);

	std::cout << "base " << Base::Base1 << std::endl;

	double dt;
	double x[3];

	std::cout << "nnz " << nnz << std::endl;
    std::cout << "m " << m << std::endl;

    bicgstab(m, nnz, A0, iA0, jA0, d_dense, x0, b_dense, 2000, 1e-5, true, x, &dt);

    std::ostringstream stream;
    dump_vector(stream, 3, x);
    dump_vector(stream, 3, d_dense);
    dump_vector(stream, 3, b_dense);

    std::cout << stream.str() << std::endl;

    free(A0);
    free(iA0);
    free(jA0);
    free(d);
    free(id);
    free(jd);
    free(b);
    free(ib);
    free(jb);

}


void test1(){
    int n;
    int m;
    int nnz;
    double *A0;
    int *iA0;
    int *jA0;
    loadMMSparseMatrix(const_cast<char *>("mat3.mtx"), 'd', true, &m, &n, &nnz, &A0, &iA0, &jA0);


    int nnz2;

    double *b;
    int *ib;
    int *jb;

    loadMMSparseMatrix(const_cast<char *>("vec3.mtx"), 'd', true, &m, &n, &nnz2, &b, &ib, &jb);

    double d_dense[3];
    double b_dense[3];

    double x0[3] = {1,1,1};




    toDenseVector(m, nnz2, b, ib, b_dense);

    std::cout << "base " << Base::Base1 << std::endl;

    double dt;
    double x[3];

    std::cout << "nnz " << nnz << std::endl;
    std::cout << "m " << m << std::endl;

    double dtAlg;
    bicgstab_lu_precond(m, nnz, A0, iA0, jA0, b_dense, 200, 1e-5, true, x, &dtAlg);

    std::ostringstream stream;
    dump_vector(stream, 3, x);
    dump_vector(stream, 3, d_dense);
    dump_vector(stream, 3, b_dense);

    std::cout << stream.str() << std::endl;

    free(A0);
    free(iA0);
    free(jA0);
    free(b);
    free(ib);
    free(jb);

}

/*int main(int argc, char *argv[]){
	return 0;
}*/

int main (int argc, char *argv[]){
    int status = EXIT_FAILURE;
    char *matrix_filename = NULL;
	char *vector_filename = NULL;
    bool debug=false;
    double prob_of_zero_mat = 0.99;
    double prob_of_zero_vec = 0.2;
    int dim = 10000;
	bool print = false;


	const int maxit = 2000;
    const double tol= 1e-6;



    /* WARNING: it is assumed that the matrices are stores in Matrix Market format */
    printf("WARNING: it is assumed that the matrices are stored in Matrix Market format with double as element type\n Usage: ./BiCGStab -M[matrix.mtx] -V[vector.mtx] [-D] -R[prob of zero] -N[dim] [-P] [device=<num>]\n"
		   "By default matrix will be random, N = 10000, P(X = 0)=0.99, vector will be random, P(X = 0)=0.1\n"
           "example usage:\n"
           "./example.exe -M\"mat10000.mtx\"\n"
           "./example.exe -M\"mat3.mtx\" -V\"vec3.mtx\" -D -P\n"
		   "./example.exe -N\"40\" -R\"0.5\" -D\n"
		   );

    int i=0;
    int temp_argc = argc;
    while (argc) {
        if (*argv[i] == '-') {
            switch (*(argv[i]+1)) { 
            case 'M':
                matrix_filename = argv[i]+2;  
                break;
			case 'V':
				vector_filename = argv[i] + 2;
				break;
            case 'D':
                debug = true;
                break;    
			case 'R':
				prob_of_zero_mat = std::stod(argv[i] + 2);
				break;
			case 'P':
				print = true;
				break;
			case 'N':
				dim = std::stoi(argv[i] + 2);
				break;
            default:
                fprintf (stderr, "Unknown switch '-%s'\n", argv[i]+1);
                return status;
            }
        }
        argc--;
        i++;
    }

    argc = temp_argc;

    if (matrix_filename != NULL){
		printf("Using matrix input file [%s]\n", matrix_filename);
    }


	if (vector_filename != NULL) {
		printf("Using vector input file [%s]\n", vector_filename);
	}


    findCudaDevice(argc, (const char **)argv);

	int n;
	int nnz;
	double *A;
	int *iA;
	int *jA;
	double *b = nullptr;
	double *x = nullptr;

	if (matrix_filename != nullptr){

		int matrixN;
		int matrixM;

		if (loadMMSparseMatrix(matrix_filename, 'd', true, &matrixM, &matrixN, &nnz, &A, &iA, &jA)) {
			fprintf(stderr, "!!!! cusparseLoadMMSparseMatrix FAILED\n");
			return EXIT_FAILURE;
		}

		if(matrixN != matrixM){
			fprintf(stderr, "!!!! square matrix is expected\n");
			return EXIT_FAILURE;
		}

		n = matrixN;


	}else{

		std::vector<double> _A;
		std::vector<int> _IA;
		std::vector<int> _JA;



		//nnz = gen_rand_csr_matrix<Base::Base1>(dim, dim, &_A, &_IA, &_JA, prob_of_zero_mat, 1.0, 5.0, 1e-2);
		nnz = fill_csr_matrix<Base::Base1>(dim, dim, &_A, &_IA, &_JA, [&](int i, int j){
		     if(i == j) return rand_float(1, 10); //A[i,i] is never zero
		     else {
		         if(rand_float_0_1() >= prob_of_zero_mat){
                     return rand_float(1, 10);
		         }else{
		             return 0.0;
		         }
		     }

		    }
		    , 1e-3);
		n = dim;



		A = static_cast<double *>(malloc(sizeof(double) * nnz));
		iA = static_cast<int *>(malloc(sizeof(int) * (n + 1)));
		jA = static_cast<int *>(malloc(sizeof(int) * nnz));

		if(_A.empty()){
			fprintf(stderr, "!!!! all random elements of the random matrix are zeros !\n");
			return EXIT_FAILURE;
		}

		memcpy(A, &_A[0], sizeof(double)*nnz);
		memcpy(iA, &_IA[0], sizeof(int)*(n + 1));
		memcpy(jA, &_JA[0], sizeof(int)*nnz);


	}

	if(vector_filename != nullptr){

		int vN;
		int vM;
		int vnnz;
		double *vA = nullptr;
		int* vIA = nullptr;
		int* vJA = nullptr;

		if (loadMMSparseMatrix(vector_filename, 'd', true, &vM, &vN, &vnnz, &vA, &vIA, &vJA)) {
			fprintf(stderr, "!!!! cusparseLoadMMSparseMatrix FAILED\n");
			return EXIT_FAILURE;
		}

		if (vN != 1) {
			fprintf(stderr, "b must be a vector !\n");
			return EXIT_FAILURE;
		}

		if (vM != n) {
			fprintf(stderr, "incorrect dim\n");
			return EXIT_FAILURE;
		}

		b = (double*)malloc(sizeof(double) * n);

		toDenseVector(vM, vnnz, vA, vIA, b);

		free(vA);
		free(vIA);
		free(vJA);
	}else{
		b = (double*)malloc(sizeof(double) * n);
		gen_rand_vector(n, b, prob_of_zero_vec, 1, 5.0);

	}

	x = static_cast<double *>(malloc(sizeof(double) * n));

	std::cout << "nnz=" << nnz << std::endl;




	double dtAlg;
	auto t1 = second();
	bool solved = bicgstab_lu_precond(n, nnz, A, iA, jA, b, maxit, tol, debug, x, &dtAlg);
	auto t2 = second();

	if(solved){
		std::cout << "success" << std::endl;
		if(print){
			std::cout << "result:" << std::endl;
			std::ostringstream s;
			dump_vector(s, n, x);
			std::cout << s.str() << std::endl;
		}

		std::cout << "algorithm delta time = " << dtAlg << " s" << std::endl;
		std::cout << "total delta time = " << t2 - t1 << " s" << std::endl;
	}else{
		std::cerr << "method failed" << std::endl;
	}

	free(x);
	free(b);
	free(A);
	free(iA);
	free(jA);


    return status;
}

