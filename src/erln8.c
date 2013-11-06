#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/param.h>

// GIO stuff
#include <glib-object.h>
#include <gio/gio.h>
#include <sys/param.h>


/*
 * TODO:
 *   free all GErrors (and... everything else)
 *   error checking for all calls
 *   g_strfreev(keys);
 *   don't hardcode my paths in the default config :-)
 */

#define G_LOG_DOMAIN    ((gchar*) 0)
#define SOURCES "sources"
#define MD5     "MD5"
static gboolean opt_init_erln8 = FALSE;
static gboolean opt_debug      = FALSE;
static gchar*   opt_use        = NULL;
static gboolean opt_list       = FALSE;
static gboolean opt_fetch      = FALSE;
static gboolean opt_build      = FALSE;
static gboolean opt_show       = FALSE;

static const gchar* homedir;
//static unsigned int last_pcnt = 0;

static GOptionEntry entries[] =
{
  { "init", 'i', 0, G_OPTION_ARG_NONE, &opt_init_erln8, "Initialize Erln8", NULL },
  { "debug", 'D', 0, G_OPTION_ARG_NONE, &opt_debug, "Debug Erln8", NULL },
  { "use", 'u', 0, G_OPTION_ARG_STRING, &opt_use, "Setup Erlang version in cwd", NULL },
  { "list", 'l', 0, G_OPTION_ARG_NONE, &opt_list, "List available Erlang installations", NULL },
  { "fetch", 'f', 0, G_OPTION_ARG_NONE, &opt_fetch, "Update source repos", NULL },
  { "build", 'b', 0, G_OPTION_ARG_NONE, &opt_build, "Build a specific version of OTP from source", NULL },
  { "show", 's', 0, G_OPTION_ARG_NONE, &opt_show, "Show the configured version of Erlang", NULL },
  { NULL }
};

void erln8_log( const gchar *log_domain,
                GLogLevelFlags log_level,
                const gchar *message,
                gpointer user_data ) {
  if(opt_debug) {
    printf("%s",message);
  }
  return;
}

// TODO: move glib logging
// also, I always hated the critical output from glib
void erln8_error(char *msg) {
  printf("ERROR: %s\n", msg);
}

void erln8_error_and_exit(char *msg) {
  printf("ERROR: %s\n", msg);
  exit(-1);
}



gboolean erl_on_path() {
  gchar *out;
  gchar *err;
  gint   status;
  GError *error;
  g_spawn_command_line_sync ("which erl", &out, &err, &status, &error);
  if(!status) {
    return 1;
  } else {
    return 0;
  }
}

char* load_config() {
  }

gchar* get_configdir_file_name(char* filename) {
  gchar *configfilename = g_strconcat(homedir, "/.erln8.d/", filename, (char*)0);
  return configfilename;
}

gchar* get_config_subdir_file_name(char *subdir, char* filename) {
  gchar *configfilename = g_strconcat(homedir, "/.erln8.d/", subdir, "/", filename, (char*)0);
  return configfilename;
}

/*
void build_erlang() {
  char *fname = "otp_src_R16B02.tar.gz";
  char *fn = get_configdir_file_name("sources","otp_src_R16B02.tar.gz");
  // TODO: check MD5
  gchar *cmd = g_strconcat("tar xf ", fn, " --strip-components 1 -C ", homedir, "/.erln8.d/otps/default/", (char*)0);
  gchar *in;
  gchar *out;
  gint status;
  GError *err;
  printf("Uncompressing sources\n");
  gboolean result = g_spawn_command_line_sync(cmd, &in, &out, &status, &err);
  g_free(cmd);
  g_free(fn);
}
*/

gint git_command(char *command) {
  gchar *cmd = g_strconcat("git ", command, NULL);
  gchar *in;
  gchar *out;
  gint status;
  GError *err;
  //printf("cmd: %s\n", cmd);
  gboolean result = g_spawn_command_line_sync(cmd, &in, &out, &status, &err);
  g_free(cmd);
  if(err != NULL) {
    printf("ERROR: %s\n", err->message);
  }
  // TODO: free in/out?
  printf("Output: %s\n", in);
  return result;
}


gboolean check_home() {
  gchar *configdir = g_strconcat(homedir, "/.erln8.d", (char*)0);
  g_debug("Checking config dir %s\n", configdir);
  //g_path_get_basename ()
  gboolean result = g_file_test(configdir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR);
  free(configdir);
  return result;
}

void mk_config_subdir(char *subdir) {
  gchar* dirname = g_strconcat(homedir, "/.erln8.d/", subdir, (char*)0);
  g_debug("Creating %s\n", dirname);
  if(g_mkdir(dirname, S_IRWXU)) {
    g_free(dirname);
    erln8_error_and_exit("Can't create directory");
    return;
  } else {
    g_free(dirname);
  }
}


void build_erlang(char *repo, char *tag, char *config) {
  // TODO: also check config_env
}

void init_main_config() {
  GKeyFile *kf = g_key_file_new();
  g_key_file_set_string(kf,
                        "Repos",
                        "default",
                        "https://github.com/erlang/otp.git");

  g_key_file_set_string(kf,
                        "Repos",
                        "basho",
                        "https://github.com/basho/otp.git");


  g_key_file_set_string(kf,
                        "Configs",
                        "osx_llvm",
                        "--disable-hipe --enable-smp-support --enable-threads --enable-kernel-poll --enable-darwin-64bit");

  g_key_file_set_string(kf,
                        "Configs",
                        "osx_gcc",
                        "--disable-hipe --enable-smp-support --enable-threads --enable-kernel-poll --enable-darwin-64bit");

  g_key_file_set_string(kf,
                        "Configs",
                        "osx_gcc_env",
                        "CC=gcc-4.2 CPPFLAGS='-DNDEBUG' MAKEFLAGS='-j 3'");

  g_key_file_set_string(kf,
                        "Erlangs",
                        "R15B01p",
                        "/Users/dparfitt/erlang_R15B01p");

  g_key_file_set_string(kf,
                        "Erlangs",
                        "R15B01",
                        "/Users/dparfitt/erlang_R15B01");

  g_key_file_set_string(kf,
                        "Erlangs",
                        "R16B02",
                        "/Users/dparfitt/erlang-R16B02");

  GError *error;
  gchar* d = g_key_file_to_data (kf, NULL, &error);
  gchar* fn = get_configdir_file_name("config");
  printf("Writing to %s\n", fn);  
  GError *error2;
  if(!g_file_set_contents(fn, d, -1, &error2)) {
    printf("Error writing config file :-(\n");
  }
  printf("DATA = %s\n", d);
  free(fn);
  g_key_file_free(kf);
}


void init_here(char* erlang) {
  GKeyFile *kf = g_key_file_new();
  g_key_file_set_string(kf,
                        "Config",
                        "Erlang",
                        erlang);

  GError *error = NULL;
  gchar* d = g_key_file_to_data (kf, NULL, &error);
  gchar* fn = "./erln8.config";
  printf("Writing to %s\n", fn);  

  GError *error2 = NULL;
  if(!g_file_set_contents(fn, d, -1, &error2)) {
    printf("Error writing config file :-(\n");
    if (error2 != NULL) {
      fprintf (stderr, "Unable to read file: %s\n", error2->message);
    }
  }
  // TODO
  //g_key_file_free(kf);
}


void list_erlangs() {
  GKeyFile *kf = g_key_file_new();
  GError *error;
  GError *error2;
  gsize keycount;
  gchar* fn = get_configdir_file_name("config");
  if(g_key_file_load_from_file(kf, fn, G_KEY_FILE_NONE, &error)) {
    printf("Available Erlang installations:\n");
    gchar** keys = g_key_file_get_keys(kf, "Erlangs", &keycount, &error2);
    int i = 0;
    for(i = 0; i < keycount; i++) {
      printf("  %s\n",*keys++);
    }
    // TODO
    //g_strfreev(keys);
  }
  // TODO: free error
  free(fn);
  g_key_file_free(kf);
}


void initialize() {
  if(check_home()) {
    erln8_error_and_exit("Configuration directory ~/.erln8.d already exists");
  //} //else if(erl_on_path()) {
    //erln8_error_and_exit("Erlang already exists on the current PATH");
  } else {
    // create the top level config directory, then create all subdirs
    gchar* dirname = g_strconcat(homedir, "/.erln8.d",(char*)0);
    g_debug("Creating %s\n", dirname);
    if(g_mkdir(dirname, S_IRWXU)) {
      erln8_error_and_exit("Can't create directory");
      g_free(dirname);
      return;
    } else {
      g_free(dirname);
    }
    mk_config_subdir("otps");    // location of compiled otp source files
    mk_config_subdir("logs");    // logs!
    mk_config_subdir("repos"); // location of git repos
    mk_config_subdir("repos/github_otp"); // location of git repos
    init_main_config();
  }
}

char* configcheck(char *d) {
  char *retval = NULL;
  char *f = g_strconcat(d, "/erln8.config", NULL);
  GFile *gf = g_file_new_for_path(f);
  GFile *gd = g_file_new_for_path(d);

  if(g_file_query_exists(gf, NULL)) {
      char *cf = g_file_get_path(gf);
      //g_free(cf);
      retval = cf;
  } else {
      if(g_file_has_parent(gd, NULL)) {
        GFile *parent = g_file_get_parent(gd);
        char *pp = g_file_get_path(parent);
        retval = configcheck(pp);
        g_object_unref(parent);
        g_free(pp);
      }
  }

  g_free(f);
  g_object_unref(gf);
  g_object_unref(gd);
  return retval;
}

char* configcheckfromcwd() {
  char *d = getcwd(NULL, MAXPATHLEN);
  char *retval = configcheck(d);
  free(d);
  return retval;
}


// TODO: this function leaks badly!
char* which_erlang() {
  char* cfgfile = configcheckfromcwd();
  // TODO: free
  if(g_file_test(cfgfile, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
    GKeyFile* kf = g_key_file_new();
    GError* err;
    // TODO: free kf
    // TODO: free err
    gboolean b = g_key_file_load_from_file(kf, cfgfile, G_KEY_FILE_NONE, &err);
    if(!g_key_file_has_group(kf, "Config")) {
      erln8_error_and_exit("erln8 Config group not defined in erln8.config\n");
      return NULL;
    } else {
      if(g_key_file_has_key(kf, "Config", "Erlang", &err)) {
        gchar* erlversion = g_key_file_get_string(kf, "Config", "Erlang", &err);
        // THIS VALUE MUST BE FREED
        return erlversion;
      } else {
        erln8_error_and_exit("Missing Erlang | version\n");
        return NULL;
      }
    }
  } else {
    erln8_error_and_exit("Config file does not exist\n");
    return NULL;
  }
}

char *get_linked_path(char *id) {
  gchar* cfgfile = get_configdir_file_name("config");
  GKeyFile* kf = g_key_file_new();
  GError* err;
  // TODO: free kf
  // TODO: free err
  gboolean b = g_key_file_load_from_file(kf, cfgfile, G_KEY_FILE_NONE, &err);

  // TODO: check for an exact match
  gchar* erlpath = g_key_file_get_string(kf, "Erlangs", id, &err);
  free(cfgfile);
  return erlpath;
}



int erln8(int argc, char* argv[]) {
  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, entries, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_error("option parsing failed: %s\n", error->message);
  }

  g_log_set_handler(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,  erln8_log, NULL);

  g_debug("argv[0] = [%s]\n",argv[0]);
 
  // used for GIO
  g_type_init();

  if(opt_init_erln8) {
    initialize();
  } else {
    if(!check_home()) {
      erln8_error_and_exit("Please initialize erln8 with -i or --init");
    }
  }

  if(opt_use) {
    init_here(opt_use);
  }

  if(opt_list) {
   list_erlangs();
  }

  if(opt_fetch) {
    printf("Not implemented\n");
  }

  if(opt_build) {
    printf("Not implemented\n");
  }

  if(opt_show) {
    char* erl = which_erlang();
    printf("%s", erl);
    free(erl);
  }
  return 0;
}


int main(int argc, char* argv[]) {
  printf("erln8 v0.1\n");
  homedir = g_get_home_dir();
  g_debug("home directory = %s\n", homedir);

  if((!strcmp(argv[0], "erln8")) || (!strcmp(argv[0], "./erln8"))) {
    erln8(argc, argv);
  } else {
    char *erl = which_erlang();
    char *path = get_linked_path(erl);
    g_debug("Using erlang %s\n", erl);
    g_debug("  ->%s\n", path);

    char *s = g_strconcat(path, "/bin/", argv[0], (char*)0);
    g_debug("%s\n",s);
    // can't free s
    execv(s, argv);
  }

 }
