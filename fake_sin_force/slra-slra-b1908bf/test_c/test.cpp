/* test.c: test program for STLS 
   .\test i - test example i, 1 <= i <= 24
   .\test   - test all examples            */ 
#include <limits>
#include <stdio.h>
#include <ctime>
#include <string.h>
#include <time.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_multifit_nlin.h>
#include "slra.h"
#include "Timer.h"


void meas_time( VarproFunction &costFun,
                double &tm_func, double &tm_grad, double &tm_pjac ) {
  NLSVarproPsiXICholesky optFun(costFun, NULL);                
  gsl_vector *x = gsl_vector_alloc(optFun.getNvar());
  gsl_vector *grad = gsl_vector_alloc(optFun.getNvar());
  gsl_matrix *jacb = gsl_matrix_alloc(optFun.getNsq(), optFun.getNvar());

  

  optFun.computeDefaultx(x);
    
  Log::lprintf(Log::LOG_LEVEL_NOTIFY,"Test costfun evaluation:\n");
#define meas_op(rep, op, tm)  \
   do {                                                                     \
     Timer timer;                                                           \
     timer.start();                                                         \
     for (int i = 0; i < rep; i++) { op; }                                  \
     timer.stop();                                                          \
     tm = timer.getElapsedTime();                                           \
   } while (0)  

  double res;      
  meas_op(10, optFun.computeFuncAndGrad(x, &res, NULL), tm_func);
  meas_op(10, optFun.computeFuncAndGrad(x, NULL, grad), tm_grad);
  meas_op(1, optFun.computeFuncAndJac(x, NULL, jacb), tm_pjac);

  gsl_matrix_free(jacb);
  gsl_vector_free(x);
  gsl_vector_free(grad);
}

#define MAX_FN  60
void run_test( const char * testname, double & time, double& fmin, 
         double &fmin2, int& iter, double& diff, 
         const char *test_type, int maxiter, const char *method, 
         bool elementwise_w, int ls_correction, int silent ) {
  gsl_matrix *Rt = NULL, *R = NULL, *v = NULL, *Phi = NULL;
  gsl_matrix nullPhi = { 0, 0, 0, 0, 0, 0 };
  gsl_vector *p = NULL, *p2 = NULL;
  int rk = 1, s_N, s_q, hasR, hasPhi, hasW, m;
  FILE *file;
  char fnR[MAX_FN], fpname[MAX_FN], fRtname[MAX_FN], fsname[MAX_FN], 
       fRresname[MAX_FN], fpresname[MAX_FN], fnPhi[MAX_FN], fnW[MAX_FN];
  /* default file names */
  sprintf(fnR, "r%s.txt", testname);
  sprintf(fpname, "p%s.txt", testname);
  sprintf(fRtname, "rt%s.txt", testname);
  sprintf(fsname, "s%s.txt", testname);
  sprintf(fRresname, "res_r%s.txt", testname);
  sprintf(fpresname, "res_p%s.txt", testname);
  sprintf(fnPhi, "phi%s.txt", testname);
  sprintf(fnW, "w%s.txt", testname);

  OptimizationOptions opt;
  opt.maxiter = maxiter;
  
  if (silent == 0) {
    Log::setMaxLevel(Log::LOG_LEVEL_ITER);
  } 
  if (silent == 2) {
    Log::setMaxLevel(Log::LOG_LEVEL_OFF);
  }
  
  opt.str2Method(method);
  opt.ls_correction = ls_correction;
  SLRAObject *so = NULL;
  gsl_vector *n_l = NULL, *m_k = NULL, *w_k = NULL;

  try {
    /* Read structure  and allocate structure object */
    file = fopen(fsname, "r");    
    Log::lprintf(Log::LOG_LEVEL_NOTIFY, "Error opening file %s\n", fsname);
    fscanf(file, "%d %d %d %d", &s_N, &s_q, &m, &rk); 
    n_l = gsl_vector_alloc(s_N);
    m_k = gsl_vector_alloc(s_q);
    w_k = gsl_vector_alloc(s_q);
    gsl_vector_fscanf(file, n_l);
    gsl_vector_fscanf(file, m_k);
    fclose(file);
    hasW = ((file = fopen(fnW, "r")) != NULL);
    if (hasW) {
      gsl_vector_fscanf(file, w_k);
      fclose(file);
    } else {
      gsl_vector_set_all(w_k, 1);
    }  
     
    if (elementwise_w) {
      gsl_vector *el_wk = gsl_vector_alloc(compute_np(m_k, n_l));
      int i = 0;
      size_t T;
      gsl_vector sv;
      for (size_t l = 0; l < s_N; l++) {
        for (size_t k = 0; k < s_q; k++, i += T) {
          T = gsl_vector_get(m_k, k) + gsl_vector_get(n_l, l) - 1;
          sv = gsl_vector_subvector(el_wk, i, T).vector;
          gsl_vector_set_all(&sv, gsl_vector_get(w_k, k));
        }
      }
      gsl_vector_free(w_k);
      w_k = el_wk;
    }
    
    size_t mh_m = 0;
    for (int i = 0; i < m_k->size; i++) {
      mh_m += gsl_vector_get(m_k, i);
    }
    
      
    size_t np = compute_np(m_k, n_l);
    
    /* Compute invariants and read everything else */ 
    read_vec(p = gsl_vector_alloc(np), fpname);
    p2 = gsl_vector_alloc(np);
    hasR = read_mat(R = gsl_matrix_calloc(m, m - rk), fnR);
    hasPhi = read_mat(Phi = gsl_matrix_alloc(mh_m, m), fnPhi);
    read_mat(Rt = gsl_matrix_calloc(m, m - rk), fRtname);
    /* call slra */  

    double rk_double = rk;
    gsl_vector rk_vec = {1,1, &rk_double, 0, 0};
    so = new SLRAObject(*p, *m_k, *n_l, (hasPhi ? *Phi : nullPhi), *w_k, rk_vec);

    if (test_type[0] == 'd') {
      so->optimize(&opt, (hasR ? R : NULL), NULL, p2, R, v);
      gsl_matrix_fprintf(file = fopen(fRresname,"w"), R, "%.14f");
      fclose(file);
      gsl_vector_fprintf(file = fopen(fpresname, "w"), p2, "%.14f");
      fclose(file);
      time = opt.time;
      iter = opt.iter;
      gsl_matrix_sub(Rt, R);
      gsl_vector Rvec = 
          gsl_vector_view_array(Rt->data, Rt->size1 * Rt->size2).vector;
      gsl_vector_sub(p2, p);
      double dp_norm = gsl_blas_dnrm2(p2);
      diff = gsl_blas_dnrm2(&Rvec);
      fmin = opt.fmin;
      fmin2 = dp_norm * dp_norm;
    } else {
      meas_time(*so->getF(),  fmin, fmin2, diff);
    }          

    throw 0;
  } catch(...) {
    gsl_vector_free_ifnull(n_l);  
    gsl_vector_free_ifnull(w_k);  
    gsl_vector_free_ifnull(m_k);  
    gsl_matrix_free_ifnull(R);
    gsl_matrix_free_ifnull(Rt);
    gsl_matrix_free_ifnull(v);
    gsl_vector_free_ifnull(p);
    gsl_vector_free_ifnull(p2);
    gsl_matrix_free_ifnull(Phi);
    if (so != NULL) {
      delete so;
    }
    Log::deleteLog();
  }
}

void print_hr( int num ) {
  while (--num >= 0) {
    printf("-");
  }
  printf("\n");
}

#define TMAX 1000

int main(int argc, char *argv[])
{
  double times[TMAX], misfits[TMAX], misfits2[TMAX], diffs[TMAX];
  int iters[TMAX];
  char num[10];
  
  if (argc < 2) {
    printf("Error: no parameters.\n"
      "Usage: %s <start_no> [<end_no> <test_type> <maxiter> <method> " 
                            "<elementwise_w> <ls_correction>]\n"
      "start_no      - starting test #, in [0;%d]\n"
      "end_no        - end test #, in [start_no--%d] (default start_no)\n"           
      "test_type     - 'd' for differences (default), 's' for speed\n"           
      "maxiter       - opt.maxiter (default 500)\n"           
      "method        - opt.method (default \"l\")\n"           
      "elementwise_w - 0 for MosaicHStructure (default), 1 for WMosaic...\n"           
      "ls_correction - opt.ls_correction (default 0)\n" 
      "silent        - log level, 0=full (default), 1=results, 2=off\n", 
      argv[0], TMAX-1, TMAX-1);
    return -1;
  }
  
  int start_no, end_no, i;
  start_no = atoi(argv[1]);
  if (start_no < 0 || start_no >= TMAX) {
    printf("Error: incorrect start_no\n");
    return -1;
  }
  end_no = argc > 2 ? atoi(argv[2]) : start_no;
  if (end_no < start_no || end_no >= TMAX) {
    printf("Error: incorrect end_no\n");
    return -1;
  }
  const char *test_type = (argc > 3 && argv[3][0] == 's' ? "s" : "d");
  int maxiter = argc > 4 ? atoi(argv[4]) : 500;
  const char *method = (argc > 5 ? argv[5] : "l");
  bool elementwise_w = argc > 6 ? (bool)atoi(argv[6]) : false;
  int ls_correction = argc > 7 ? atoi(argv[7]) : 0;
  int silent = argc > 8 ? atoi(argv[8]) : 0;

  if (silent != 2) {
    printf("\n---------------- Testing examples %3d-%3d -----------------\n", 
         start_no, end_no);
  }
  for( i = start_no; i <= end_no; i++ ) {
    sprintf(num, "%d", i);
    if (silent != 2) {
      printf("Running test %s\n", num);  
    }
    times[i] =  misfits[i] = misfits2[i] = diffs[i] = iters[i] = 0;
    run_test(num, times[i], misfits[i], misfits2[i],  iters[i], diffs[i], 
             test_type,  maxiter, method, elementwise_w, ls_correction, silent);
    if (silent != 2) {
      printf("Result: (time, fmin, diff) = %10.8f %10.8f %f\n",
             times[i], misfits[i], diffs[i]);
    }
  } 

  if (silent != 2) {
    printf("\n------------ Results summary --------------------\n\n");
    print_hr(72);
  }
  printf("  no         Time   Iter   %s   %s   %s  \n",
         (test_type[0] == 'd' ? "    Minimum" : "     t_func"), 
         (test_type[0] == 'd' ? "  Minimum_2" : "     t_grad"), 
         (test_type[0] == 'd' ? "         Diff_R" : "         t_pjac"));
 
  for( i = start_no; i <= end_no; i++ ) {
    printf("  %2d   %10.6f   %4d   %11.7f   %11.7f   %15.10f  \n", 
        i, times[i], iters[i], misfits[i], misfits2[i], diffs[i], method);
  }
  if (silent != 2) {
    print_hr(72);
  }

  return 0;
}
