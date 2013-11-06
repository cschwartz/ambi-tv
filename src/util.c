/* ambi-tv: a flexible ambilight clone for embedded linux
*  Copyright (C) 2013 Georg Kaindl
*  
*  This file is part of ambi-tv.
*  
*  ambi-tv is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 2 of the License, or
*  (at your option) any later version.
*  
*  ambi-tv is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  
*  You should have received a copy of the GNU General Public License
*  along with ambi-tv.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "log.h"

#define LOGNAME            "util: "

#define LIST_GROW_STEP     4

int
ambitv_util_append_ptr_to_list(void*** list_ptr, int idx, int* len_ptr, void* ptr)
{   
   if (NULL == *list_ptr) {
      *list_ptr = (void**)malloc(sizeof(void*) * LIST_GROW_STEP);
      *len_ptr = LIST_GROW_STEP;
   } else if (idx >= *len_ptr) {
      *len_ptr += LIST_GROW_STEP;
      *list_ptr = (void**)realloc(*list_ptr, sizeof(void*) * (*len_ptr));
   }

   (*list_ptr)[idx] = ptr;

   return ++idx;
}

int ambitv_util_status_update(char *status_file, char *status_content)
{
  int ret = 0;
  FILE *status_fid = NULL;
  char *full_path = NULL;

  ret = asprintf(&full_path, "%s/%s", STATUS_PATH, status_file);
  if(ret < 0) {
    goto errReturn;
  }

  status_fid = fopen(full_path, "w");
  free(full_path);
  full_path = NULL;

  if(status_fid == 0) {
     ret = -1;
     goto errReturn;
  }
 
  fprintf(status_fid, "%s", status_content);

  errReturn:
    if(status_fid > 0)
      fclose(status_fid);

    return ret;
}

int
ambitv_parse_led_string(const char* str, int** out_ptr, int* out_len)
{
   int a, b, c, ilen = 0, idx = 0, skip = 0, *s = &a;
   int* list = NULL;
   
   a = b = 0;
   while (1) {
      c = *str++;
      
      switch(c) {
         case 'X':
         case 'x': {
            if (&b == s) {
               ambitv_log(ambitv_log_error, LOGNAME "unexpected '%c'.\n",
                  c);
               goto errReturn;
            }
            
            skip = 1;            
            break;
         }
         
         case '-': {            
            if (&a == s && !skip) {
               s = &b;
            } else {
               ambitv_log(ambitv_log_error, LOGNAME "unexpected '%c'.\n",
                  c);
               goto errReturn;
            }
            
            break;
         }
         
         case '\0':
         case '\n':
         case ',': {            
            if (skip) {
               while (a--)
                  idx = ambitv_util_append_ptr_to_list(
                     (void***)&list,
                     idx,
                     &ilen,
                     (void*)-1
                  );
            } else {         
               int l = a, r = a+1, ss = 1;
               
               if (&b == s) {
                  ss = (l > b) ? -1 : 1;
                  r = b+ss;
               }
               
               do {               
                  idx = ambitv_util_append_ptr_to_list(
                     (void***)&list,
                     idx,
                     &ilen,
                     (void*)l
                  );
                  
                  l += ss;
               } while (l != r);
            }
            
            skip = a = b = 0;
            s = &a;
            
            break;
         }
         
         default: {
            if (skip || c < '0' || c > '9') {
               ambitv_log(ambitv_log_error, LOGNAME "unexpected character '%c'.\n",
                  c);
               goto errReturn;
            }
            
            *s = *s * 10 + (c-'0');
            break;
         }
      }
      
      if (c == '\0' || c == '\n')
         break;
   }
   
   *out_ptr = list;
   *out_len = idx;
      
   return 0;

errReturn:
   return -1;
}
