int petegg_portable_compile_smoke(void);
int petegg_jieli_port_smoke(void);
int petegg_app_main_smoke(void);

int petegg_app_smoke_run(void) {
  if (petegg_portable_compile_smoke() != 0) {
    return -1;
  }
  if (petegg_jieli_port_smoke() != 0) {
    return -2;
  }
  if (petegg_app_main_smoke() != 0) {
    return -3;
  }
  return 0;
}
