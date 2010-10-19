#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main()
{
  // Attempt to dynamically load the python module,
  // attempting to resolve all symbols immediately:
  const char* path = "glom/python_embed/python_module/.libs/glom_1.20.so";
  void* lib = dlopen(path, RTLD_NOW);

  if(!lib)
  {
    char *error = dlerror();
    if(error)
    {
      fprintf (stderr, "%s\n", error);
    }

    return EXIT_FAILURE;
  }
  else
    dlclose(lib);

  return EXIT_SUCCESS;
}
