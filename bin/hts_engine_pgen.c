/* ----------------------------------------------------------------- */
/*           The HMM-Based Speech Synthesis Engine "hts_engine API"  */
/*           developed by HTS Working Group                          */
/*           http://hts-engine.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2001-2015  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/*                2001-2008  Tokyo Institute of Technology           */
/*                           Interdisciplinary Graduate School of    */
/*                           Science and Engineering                 */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef HTS_ENGINE_P_GEN_C
#define HTS_ENGINE_P_GEN_C

#ifdef __cplusplus
#define HTS_ENGINE_P_GEN_C_START extern "C" {
#define HTS_ENGINE_P_GEN_C_END   }
#else
#define HTS_ENGINE_P_GEN_C_START
#define HTS_ENGINE_P_GEN_C_END
#endif                          /* __CPLUSPLUS */

HTS_ENGINE_P_GEN_C_START;

#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "HTS_engine.h"

/* usage: output usage */
void usage(void)
{
   fprintf(stderr, "%s\n", HTS_COPYRIGHT);
   fprintf(stderr, "hts_engine - The HMM-based speech synthesis engine \"hts_engine API\"\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "    hts_engine [ options ] [ infile ]\n");
   fprintf(stderr, "  options:                                                                   [  def][ min-- max]\n");
   fprintf(stderr, "    -m  htsvoice   : HTS voice files                                         [  N/A]\n");
   fprintf(stderr, "    -ot s          : filename of output trace information                    [  N/A]\n");
   fprintf(stderr, "    -vp            : use phoneme alignment for duration                      [  N/A]\n");
   fprintf(stderr, "    -r  f          : speech speed rate                                       [  1.0][ 0.0--    ]\n");
   fprintf(stderr, "    -fm f          : additional half-tone                                    [  0.0][    --    ]\n");
   fprintf(stderr, "    -u  f          : voiced/unvoiced threshold                               [  0.5][ 0.0-- 1.0]\n");
   fprintf(stderr, "    -g  f          : volume (dB)                                             [  0.0][    --    ]\n");
   fprintf(stderr, "  infile:\n");
   fprintf(stderr, "    label file\n");
   fprintf(stderr, "  output_directory:\n");
   fprintf(stderr, "    the output directory\n");
   fprintf(stderr, "  note:\n");
   fprintf(stderr, "    generated sequences are saved in natural endian, binary (float) format.\n");
   fprintf(stderr, "\n");

   exit(0);
}

int main(int argc, char **argv)
{
   int i;
   double f;
   int nstream;

   /* hts_engine API */
   HTS_Engine engine;

   /* HTS voices */
   size_t num_voices;
   char **fn_voices;

   /* input label file name */
   char *labfn = NULL;

   /* temporary output file name */
   char *output_directory = NULL;
   char *tmpfn = NULL;

   /* output file pointers */
   FILE *tmpfp, *tracefp = NULL;

   /* interpolation weights */
   size_t num_interpolation_weights;

   /* output usage */
   if (argc <= 1)
      usage();

   /* initialize hts_engine API */
   HTS_Engine_initialize(&engine);

   /* get HTS voice file names */
   num_voices = 0;
   fn_voices = (char **) malloc(argc * sizeof(char *));
   for (i = 0; i < argc; i++) {
      if (argv[i][0] == '-' && argv[i][1] == 'm')
         fn_voices[num_voices++] = argv[++i];
      if (argv[i][0] == '-' && argv[i][1] == 'h')
         usage();
   }
   if (num_voices == 0) {
      fprintf(stderr, "Error: HTS voice must be specified.\n");
      free(fn_voices);
      exit(1);
   }

   /* load HTS voices */
   if (HTS_Engine_load(&engine, fn_voices, num_voices) != TRUE) {
      fprintf(stderr, "Error: HTS voices cannot be loaded.\n");
      free(fn_voices);
      HTS_Engine_clear(&engine);
      exit(1);
   }
   free(fn_voices);

   /* get options */
   while (--argc) {
      if (**++argv == '-') {
         switch (*(*argv + 1)) {
         case 'v':
            switch (*(*argv + 2)) {
            case 'p':
               HTS_Engine_set_phoneme_alignment_flag(&engine, TRUE);
               break;
            default:
               fprintf(stderr, "Error: Invalid option '-v%c'.\n", *(*argv + 2));
               HTS_Engine_clear(&engine);
               exit(1);
            }
            break;
         case 'o':
            switch (*(*argv + 2)) {
            case 't':
               tracefp = fopen(*++argv, "wt");
               break;
            default:
               fprintf(stderr, "Error: Invalid option '-o%c'.\n", *(*argv + 2));
               HTS_Engine_clear(&engine);
               exit(1);
            }
            --argc;
            break;
         case 'h':
            usage();
            break;
         case 'm':
            argv++;             /* HTS voices were already loaded */
            --argc;
            break;
         case 'r':
            HTS_Engine_set_speed(&engine, atof(*++argv));
            --argc;
            break;
         case 'f':
            switch (*(*argv + 2)) {
            case 'm':
               HTS_Engine_add_half_tone(&engine, atof(*++argv));
               break;
            default:
               fprintf(stderr, "Error: Invalid option '-f%c'.\n", *(*argv + 2));
               HTS_Engine_clear(&engine);
               exit(1);
            }
            --argc;
            break;
         case 'u':
            HTS_Engine_set_msd_threshold(&engine, 1, atof(*++argv));
            --argc;
            break;
         case 'i':
            num_interpolation_weights = atoi(*++argv);
            argc--;
            if (num_interpolation_weights != num_voices) {
               HTS_Engine_clear(&engine);
               exit(1);
            }
            for (i = 0; i < num_interpolation_weights; i++) {
               f = atof(*++argv);
               argc--;
               HTS_Engine_set_duration_interpolation_weight(&engine, i, f);
               HTS_Engine_set_parameter_interpolation_weight(&engine, i, 0, f);
               HTS_Engine_set_parameter_interpolation_weight(&engine, i, 1, f);
               HTS_Engine_set_gv_interpolation_weight(&engine, i, 0, f);
               HTS_Engine_set_gv_interpolation_weight(&engine, i, 1, f);
            }
            break;
         case 'j':
            switch (*(*argv + 2)) {
            case 'm':
               HTS_Engine_set_gv_weight(&engine, 0, atof(*++argv));
               break;
            case 'f':
            case 'p':
               HTS_Engine_set_gv_weight(&engine, 1, atof(*++argv));
               break;
            default:
               fprintf(stderr, "Error: Invalid option '-j%c'.\n", *(*argv + 2));
               HTS_Engine_clear(&engine);
               exit(1);
            }
            --argc;
            break;
         case 'g':
            HTS_Engine_set_volume(&engine, atof(*++argv));
            --argc;
            break;
         default:
            fprintf(stderr, "Error: Invalid option '-%c'.\n", *(*argv + 1));
            HTS_Engine_clear(&engine);
            exit(1);
         }
      } else {
        labfn = argv[argc-2];
        output_directory = argv[argc-1];
      }
   }

   /* Generate state sequence */
   if (HTS_Engine_generate_state_sequence_from_fn(&engine, labfn) != TRUE) {
      fprintf(stderr, "Error: state sequence can't be generated.\n");
      HTS_Engine_clear(&engine);
      exit(1);
   }

   /* Generate parameter sequence */
   if (HTS_Engine_generate_parameter_sequence(&engine) != TRUE) {
      fprintf(stderr, "Error: parameter sequence can't be generated.\n");
      HTS_Engine_clear(&engine);
      exit(1);
   }

   if (HTS_Engine_MinimalGStreamSet_create(&engine.gss, &engine.pss, engine.condition.fperiod) != TRUE) {
      fprintf(stderr, "Error: parameter sequence can't be generated. (step 2)\n");
      HTS_Engine_clear(&engine);
      exit(1);
   }

   /* Save duration */
   tmpfn = malloc(sizeof(char) * (strlen(output_directory) + strlen(labfn)));
   sprintf(tmpfn, "%s/%s.%s", output_directory, basename(labfn), "dur");
   if ((tmpfp = fopen(tmpfn, "wt")) <= 0) {
     fprintf(stderr, "couldn't open \"%s\": ", tmpfn);
     perror("");
     exit(EXIT_FAILURE);
   }
   HTS_Engine_save_label(&engine, tmpfp);
   fclose(tmpfp);

   /* Save parameters */
   nstream = HTS_Engine_get_nstream(&engine);
   for (i=0; i<nstream; i++) {
     sprintf(tmpfn, "%s/%s.%d", output_directory, basename(labfn), i);
     if ((tmpfp = fopen(tmpfn, "wb")) <= 0) {
       fprintf(stderr, "couldn't open \"%s\": ", tmpfn);
       perror("");
       exit(EXIT_FAILURE);
     }
     HTS_Engine_save_generated_parameter(&engine, i, tmpfp);
     fclose(tmpfp);
   }

   /* output trace*/
   if (tracefp != NULL) {
      HTS_Engine_save_information(&engine, tracefp);
      fclose(tracefp);
   }

   /* reset */
   HTS_Engine_refresh(&engine);

   /* free memory */
   HTS_Engine_clear(&engine);

   return 0;
}

HTS_ENGINE_P_GEN_C_END;

#endif                          /* !HTS_ENGINE_P_GEN_C */
