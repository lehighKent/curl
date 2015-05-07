#include <Rinternals.h>
#include <curl/curl.h>
#include "utils.h"

int eval_callback(SEXP call) {
  int ok;
  SEXP res = PROTECT(R_tryEval(call, R_GlobalEnv, &ok));

  if (ok != 0 || pending_interrupt()) {
    UNPROTECT(1);
    return 0;
  }

  if (TYPEOF(res) != LGLSXP || length(res) != 1) {
    UNPROTECT(1);
    Rf_warning("progress callback must return boolean");
    return 0;
  }

  UNPROTECT(1);
  return asLogical(res);
}

int r_curl_callback_progress(SEXP fun,
                             double dltotal, double dlnow,
                             double ultotal, double ulnow) {

  SEXP down = PROTECT(allocVector(REALSXP, 2));
  REAL(down)[0] = dltotal;
  REAL(down)[1] = dlnow;

  SEXP up = PROTECT(allocVector(REALSXP, 2));
  REAL(up)[0] = ultotal;
  REAL(up)[1] = ulnow;

  SEXP call = PROTECT(LCONS(fun, LCONS(down, LCONS(up, R_NilValue))));
  int ok = eval_callback(call);
  UNPROTECT(3);

  return !ok;
}
