/* stringutil.c -- string utilities */

#define INCLUDE_ASCII_TABLE

#include "copyright.h"
#include "autoconf.h"

#include "mudconf.h"
#include "match.h"
#include "config.h"
#include "externs.h"
#include "alloc.h"
#include "rhost_utf8.h"
#include "rhost_ansi.h"
#include "vattr.h"

int safe_copy_buf(const char *src, int nLen, char *buff, char **bufc);
extern dbref FDECL(match_thing, (dbref, char *));
extern NAMETAB attraccess_nametab[];


/*
 * set attribs hidden
 */
void attr_wizhidden(char *str)
{
   static char x_buff[SBUF_SIZE+1], *p, *q;
   VATTR *va;

   for (p = x_buff, q = str; *q && ((p - x_buff) < (SBUF_SIZE - 1)); p++, q++)
       *p = ToLower((int)*q);
   *p = '\0';

   va = (VATTR *) vattr_find(x_buff);
   if ( va ) {
      va->flags |= (AF_WIZARD|AF_MDARK);
   }
}

void attr_internal(char *str)
{
   static char x_buff[SBUF_SIZE+1], *p, *q;
   VATTR *va;

   for (p = x_buff, q = str; *q && ((p - x_buff) < (SBUF_SIZE - 1)); p++, q++)
       *p = ToLower((int)*q);
   *p = '\0';

   va = (VATTR *) vattr_find(x_buff);
   if ( va ) {
      va->flags |= (AF_GOD|AF_DARK|AF_INTERNAL);
   }
}

void attr_generic(char *str, char *flags)
{
   static char x_buff[SBUF_SIZE+1], *p, *q;
#ifndef STANDALONE
   int i_flag, i_not;
   char *s_strtok, *s_strtokr, *s_buff;
#endif
   VATTR *va;


   for (p = x_buff, q = str; *q && ((p - x_buff) < (SBUF_SIZE - 1)); p++, q++)
       *p = ToLower((int)*q);
   *p = '\0';

   va = (VATTR *) vattr_find(x_buff);
   if ( va ) {
#ifndef STANDALONE
      s_buff = alloc_lbuf("attr_generic");
      sprintf(s_buff, "%.*s", LBUF_SIZE - 1, flags);
      for ( s_strtok = s_buff; *s_strtok; s_strtok++ ) {
         *s_strtok = ToLower((int)*s_strtok);
      }
      s_strtok = strtok_r(s_buff, " \t", &s_strtokr);
      while ( s_strtok ) {
         i_not = 0;
         if ( *s_strtok == '!' ) {
            i_not = 1;
            if ( *(s_strtok+1) ) {
               i_flag = search_nametab(GOD, attraccess_nametab, s_strtok+1);
            } else {
               i_flag = -1;
            }
         } else {
            i_flag = search_nametab(GOD, attraccess_nametab, s_strtok);
         }
         if ( i_flag >= 0 ) {
            if ( i_not ) {
               va->flags &= ~i_flag;
            } else {
               va->flags |= i_flag;
            }
         }
         s_strtok = strtok_r(NULL, " \t", &s_strtokr);
      }
      free_lbuf(s_buff);
#endif
   }
}

/*
 * returns a pointer to the non-space character in s, or a NULL if s == NULL
 * or *s == NULL or s has only spaces.
 */
char *skip_space(const char *s)
{
char *cp;

	cp = (char *)s;
	while (cp && *cp && isspace((int)*cp))
		cp++;
	return (cp);
}

/*
 * returns a pointer to the next character in s matching c, or a pointer to
 * the \0 at the end of s.  Yes, this is a lot like index, but not exactly.
 */
char *seek_char(const char *s, char c)
{
char	*cp;

	cp = (char *)s;
	while (cp && *cp && (*cp != c))
		cp++;
	return (cp);
}

/* ---------------------------------------------------------------------------
 * munge_space: Compress multiple spaces to one space, also remove leading and
 * trailing spaces.
 */

char *munge_space(char *string)
{
   char *buffer, *p, *q;

   buffer = alloc_lbuf("munge_space");
   p = string;
   q = buffer;
   while (p && *p && isspace((int)*p)) {
      p++;		/* remove initial spaces */
   }
   while (p && *p) {
      while (*p && !isspace((int)*p)) {
         *q++ = *p++;
      }
      while (*p && isspace((int)*++p)) {
         ;
      }
      if (*p) {
         *q++ = ' ';
      }
   }
   *q = '\0'; /* remove terminal spaces and terminate string */
   return (buffer);
}

/* ---------------------------------------------------------------------------
 * trim_spaces: Remove leading and trailing spaces.
 */

char *trim_spaces(char *string)
{
char	*buffer, *p, *q;

	buffer = alloc_lbuf("trim_spaces");
	p = string;
	q = buffer;
	while (p && *p && isspace((int)*p))		/* remove initial spaces */
		p++;
	while (p && *p) {
		while (*p && !isspace((int)*p))	/* copy nonspace chars */
			*q++ = *p++;
		while (*p && isspace((int)*p))	/* compress spaces */
			p++;
		if (*p) *q++ = ' ';		/* leave one space */
	}
	*q = '\0';				/* terminate string */
	return (buffer);
}

/* ---------------------------------------------------------------------------
 * grabto: Return portion of a string up to the indicated character.  Also
 * returns a modified pointer to the string ready for another call.
 */

char *grabto (char **str, char targ)
{
char	*savec, *cp;

	if (!str || !*str || !**str)
		return NULL;

	savec = cp = *str;
	while (*cp && *cp != targ)
		cp++;
	if (*cp)
		*cp++ = '\0';
	*str = cp;
	return savec;
}

int string_compare(const char *s1, const char *s2)
{
#ifndef STANDALONE
  if( !mudconf.space_compress || mudstate.no_space_compress ) {
    while (*s1 && *s2 && ToLower((int)*s1) == ToLower((int)*s2))
      s1++, s2++;

    return (ToLower((int)*s1) - ToLower((int)*s2));
  } else {
#endif
    while (isspace((int)*s1))
      s1++;
    while (isspace((int)*s2))
      s2++;
    while (*s1 && *s2 && ((ToLower((int)*s1) == ToLower((int)*s2)) ||
			  (isspace((int)*s1) && isspace((int)*s2)))) {
      if (isspace((int)*s1) && isspace((int)*s2)) {	/* skip all other spaces */
        while (isspace((int)*s1))
	  s1++;
        while (isspace((int)*s2))
	  s2++;
      } else {
        s1++;
        s2++;
      }
    }
    if ((*s1) && (*s2))
      return (1);
    if (isspace((int)*s1)) {
      while (isspace((int)*s1))
        s1++;
      return (*s1);
    }
    if (isspace((int)*s2)) {
      while (isspace((int)*s2))
        s2++;
      return (*s2);
    }
    if ((*s1) || (*s2))
      return (1);
    return (0);
#ifndef STANDALONE
  }
#endif
}

int string_prefix(const char *string, const char *prefix)
{
  int count=0;

  while (*string && *prefix && ToLower((int)*string) == ToLower((int)*prefix))
    string++, prefix++, count++;
  if (*prefix == '\0')  /* Matched all of prefix */
    return(count);
  else
    return(0);
}

/* accepts only nonempty matches starting at the beginning of a word */

const char *string_match(const char *src, const char *sub)
{
  if ((*sub != '\0') && (src)) {
    while (*src) {
      if (string_prefix(src, sub))
	return src;
      /* else scan to beginning of next word */
      while (*src && isalnum((int)*src))
	src++;
      while (*src && !isalnum((int)*src))
	src++;
    }
  }
  return 0;
}

/* ---------------------------------------------------------------------------
 * replace_string: Returns an lbuf containing string STRING with all occurrences
 * of OLD replaced by NEW. OLD and NEW may be different lengths.
 * (mitch 1 feb 91)
 */

int compare_ansibits(ANSISPLIT *sp1, ANSISPLIT *sp2, int len)
{
   ANSISPLIT *p1, *p2;
   int i_count;

   p1 = sp1;
   p2 = sp2;
   i_count = 0;
   while ( p1 && p2 && (i_count < len)) {
      i_count++;
      if ( (p1->i_ascii8 != p2->i_ascii8) ||
           (p1->i_utf8 != p2->i_utf8) ) {
         return 0;
      }
      p1++;
      p2++;
   }
   return 1;
}

#ifndef STANDALONE
char *replace_string_ansi(const char *s_old, const char *new, 
                          const char *string, int i_value, int i_flag)
{
   char	*i, *r, *s, *outbuff, *outbuff2, *inbuff, *result, *old;
   int	olen, i_once, i_count, i_olen;
   ANSISPLIT outsplit[LBUF_SIZE], outsplit2[LBUF_SIZE], *p_sp, *p_sp2, *p_old,
             insplit[LBUF_SIZE], oldsplit[LBUF_SIZE], *p_ip, *p_sptmp;

   if (string == NULL) 
      return NULL;

   initialize_ansisplitter(outsplit, LBUF_SIZE);
   initialize_ansisplitter(outsplit2, LBUF_SIZE);
   initialize_ansisplitter(insplit, LBUF_SIZE);
   initialize_ansisplitter(oldsplit, LBUF_SIZE);
   outbuff = alloc_lbuf("replace_string_ansi");
   outbuff2 = alloc_lbuf("replace_string_ansi2");
   inbuff = alloc_lbuf("replace_string_ansi3");
   old = alloc_lbuf("replace_string_ansi4");
   memset(outbuff, '\0', LBUF_SIZE);
   memset(outbuff2, '\0', LBUF_SIZE);
   memset(inbuff, '\0', LBUF_SIZE);
   memset(old, '\0', LBUF_SIZE);
   split_ansi(strip_ansi(string), outbuff, outsplit);
   split_ansi(strip_ansi(new), inbuff, insplit);
   split_ansi(strip_ansi(s_old), old, oldsplit);


   s = (char *)outbuff;
   r = (char *)outbuff2;
   p_sp = outsplit;
   p_sp2 = outsplit2;
   p_old = oldsplit;
   olen = strlen(old);
   i_once = i_count = 0;

   while (*s && (i_count < (LBUF_SIZE - 20)) ) { /* Copy up to the next occurrence of the first char of OLD */
      while (*s && (*s!=*old) && 
             (!(p_sp->i_ascii8) || (p_sp->i_ascii8 != p_old->i_ascii8)) &&
             (!(p_sp->i_utf8) || (p_sp->i_utf8 != p_old->i_utf8)) && (i_count < (LBUF_SIZE - 20)) ) {
         *r++ = *s++;         
         clone_ansisplitter(p_sp2, p_sp);
         p_sp++;
         p_sp2++;
         i_count++;
      }

      /* If we are really at an OLD, append NEW to the result and
       * bump the input string past the occurrence of OLD.
       * Otherwise, copy the char and try again.
       */
      if (*s && (i_count < (LBUF_SIZE - 20) )) {
         if (!i_once && !strncmp(old, s, olen) && compare_ansibits(p_old, p_sp, olen)) {
            i = (char *)inbuff;
            p_ip = insplit;
            p_sptmp = p_sp;
            i_olen = 0;
            while ( *i && (i_count < (LBUF_SIZE - 20)) ) {
               *r++ = *i++;
               if ( i_olen < olen ) {
                  clone_ansisplitter_two(p_sp2, p_ip, p_sp);
                  p_sp++; 
               } else {
                  if ( i_flag == 0 ) {
                     clone_ansisplitter_two(p_sp2, p_ip, p_sptmp + olen - 1);
                  } else {
                     clone_ansisplitter(p_sp2, p_ip);
                  }
               }
               p_ip++;
               p_sp2++;
               i_count++;
               i_olen++;
            }
            i_olen = 0;
            p_sp = p_sptmp;
            s += olen; 
            p_sp = p_sptmp + olen;
            if ( i_value ) {
               i_once = 1;
            }
         } else {
            *r = *s;
            clone_ansisplitter(p_sp2, p_sp);
            s++;
            r++;
            p_sp++;
            p_sp2++;
            i_count++;
         }
      }
   }
   result = rebuild_ansi(outbuff2, outsplit2, 0);
   free_lbuf(outbuff);
   free_lbuf(outbuff2);
   free_lbuf(inbuff);
   free_lbuf(old);

   return result;
}
#endif

char *replace_string(const char *old, const char *new, 
		const char *string, int i_value)
{
char	*result, *r, *s;
int	olen, i_once;

	if (string == NULL) {
           return NULL;
        } 
	s=(char *)string;
	olen = strlen(old);
	r = result = alloc_lbuf("replace_string");
        i_once = 0;
	while (*s) {

		/* Copy up to the next occurrence of the first char of OLD */

		while (*s && *s!=*old) {
			safe_chr(*s, result, &r);
			s++;
		}

		/* If we are really at an OLD, append NEW to the result and
		 * bump the input string past the occurrence of OLD.
		 * Otherwise, copy the char and try again.
		 */
			
		if (*s) {
			if (!i_once && !strncmp(old, s, olen)) {
				safe_str((char *)new, result, &r);
				s += olen;
                                if ( i_value ) 
                                   i_once = 1;
			} else {
				safe_chr(*s, result, &r);
				s++;
			}
		}
	}
	*r = '\0';
	return result;
}

/* ---------------------------------------------------------------------------
 * replace_tokens: Performs ## and #@ substitution.
 */
char *
replace_tokens(const char *s, const char *pBound, const char *pListPlace, const char *pSwitch)
{
    char *result, *r, *p;
    int n;

    if (!s) {
        return NULL;
    }
    r = result = alloc_lbuf("replace_tokens");
    while (*s) {
        // Find next '#'.
        //
        p = strchr(s, '#');
        if (p) {
            // Copy up to the next occurrence of the first character.
            //
            n = p - s;
            if (n) {
                safe_copy_buf(s, n, result, &r);
                s += n;
            }

            if (  s[1] == '#' && pBound) {
                // BOUND_VAR
                //
                safe_str((char *)pBound, result, &r);
                s += 2;
            } else if (  s[1] == '@' && pListPlace) {
                // LISTPLACE_VAR
                //
                safe_str((char *)pListPlace, result, &r);
                s += 2;
            } else if (  s[1] == '$' && pSwitch) {
                // SWITCH_VAR
                //
                safe_str((char *)pSwitch, result, &r);
                s += 2;
            } else {
                safe_chr(*s, result, &r);
                s++;
            }
        } else {
            // Finish copying source string. No matches. No further
            // work to perform.
            //
            safe_str((char *)s, result, &r);
            break;
        }
    }
    return result;
}

/*
 * Returns string STRING with all occurrences * of OLD replaced by NEW. OLD
 * and NEW may be different lengths. Modifies string, so: Note - STRING must
 * already be allocated large enough to handle the new size. (mitch 1 feb 91)
 */

char *replace_string_inplace(const char *old, const char *new, char *string)
{
char	*s;

	s = replace_string(old, new, string, 0);
	strcpy(string, s);
	free_lbuf(s);
	return string;
}

/* Counts occurrences of C in STR. - mnp 7 feb 91 */
/**************************************************************************** 
 * This is for reference only                                               * 
 ****************************************************************************
 * #define SPLIT_NORMAL            0x00
 * #define SPLIT_HILITE            0x01
 * #define SPLIT_FLASH             0x02
 * #define SPLIT_UNDERSCORE        0x04
 * #define SPLIT_INVERSE           0x08
 * #define SPLIT_NOANSI            0x10
 * #define SPLIT_FG                0x20
 * #define SPLIT_BG                0x40
 *
 * typedef struct ansisplit {
 *         char    s_fghex[5];     // Hex representation - foreground
 *         char    s_bghex[5];     // Hex representation - background 
 *         char    c_fgansi;       // Normal foreground ansi
 *         char    c_bgansi;       // Normal background ansi 
 *         int     i_special;      // Special ansi characters 
 *         char    c_accent;       // Various accent characters 
 *         int     i_ascii8;       // Extended ASCII-8 encoding %<###> foo
 * } ANSISPLIT;
 ****************************************************************************/

 /* What we want:
  * EXACT match replacement
  * JUST ansi replacement
  * JUST accent replacement
  * JUST special replacement
  * ABSCENSE OF ALL replacement
  */
void
search_and_replace_ansi(char *s_input, ANSISPLIT *a_input, ANSISPLIT *search_val, ANSISPLIT *replace_val, int i_search, int i_replace) 
{
   ANSISPLIT *s_pt, *r_pt, *a_pt;
   char *s_iptr;
   int i_mark, i_fg, i_bg;

#ifndef ZENTY_ANSI
   return;
#endif

   s_iptr = s_input;
   a_pt = a_input;
   r_pt = replace_val;
   s_pt = search_val;

   i_fg = (i_search & SPLIT_FG);
   i_search &= ~SPLIT_FG;
   i_bg = (i_search & SPLIT_BG);
   i_search &= ~SPLIT_BG;
   i_replace &= ~SPLIT_FG;
   i_replace &= ~SPLIT_BG;

   while ( s_iptr && *s_iptr ) {
      /* No Special Char Searching Here : exact match and replace */
      if ( !*(s_pt->s_fghex) && !*(a_pt->s_fghex) &&
           !*(s_pt->s_bghex) && !*(a_pt->s_bghex) &&
           !(s_pt->c_fgansi) && !(a_pt->c_fgansi) &&
           !(s_pt->c_bgansi) && !(a_pt->c_bgansi) &&
           !i_fg && !i_bg &&
           !(i_search && (a_pt->i_special & i_search)) &&
           !(s_pt->i_special) && !(a_pt->i_special) ) {
          strcpy(a_pt->s_fghex, r_pt->s_fghex); 
          strcpy(a_pt->s_bghex, r_pt->s_bghex); 
          a_pt->c_fgansi = r_pt->c_fgansi;
          a_pt->c_bgansi = r_pt->c_bgansi;
          if ( i_replace ) {
             a_pt->i_special &= ~i_replace;
             a_pt->i_special |= r_pt->i_special;
          } else {
             a_pt->i_special = r_pt->i_special;
          }
      /* Exact match searching here : exact match and replace */
      } else if ( ((*(a_pt->s_fghex) && i_fg) || (!i_fg && (strcmp(s_pt->s_fghex, a_pt->s_fghex) == 0))) &&
                  ((*(a_pt->s_bghex) && i_bg) || (!i_bg && (strcmp(s_pt->s_bghex, a_pt->s_bghex) == 0))) &&
                  (((a_pt->c_fgansi) && i_fg) || (!i_fg && (s_pt->c_fgansi == a_pt->c_fgansi))) &&
                  (((a_pt->c_bgansi) && i_bg) || (!i_bg && (s_pt->c_bgansi == a_pt->c_bgansi))) &&
                 !(i_search && (a_pt->i_special & i_search)) &&
                  (s_pt->i_special == a_pt->i_special) ) {
          strcpy(a_pt->s_fghex, r_pt->s_fghex); 
          strcpy(a_pt->s_bghex, r_pt->s_bghex); 
          a_pt->c_fgansi = r_pt->c_fgansi;
          a_pt->c_bgansi = r_pt->c_bgansi;
          if ( i_replace ) {
             a_pt->i_special &= ~i_replace;
             a_pt->i_special |= r_pt->i_special;
          } else {
             a_pt->i_special = r_pt->i_special;
          }
      /* Just match if ANSI fg hex : fg to fg */
      } else if ( ((*(a_pt->s_fghex) && i_fg) || (*(s_pt->s_fghex) && (strcmp(s_pt->s_fghex, a_pt->s_fghex) == 0))) &&
                  !*(s_pt->s_bghex) &&
                  !(s_pt->c_fgansi) &&
                  !(s_pt->c_bgansi) &&
                  !(i_search && (a_pt->i_special & i_search)) &&
                  !(s_pt->i_special) ) {
          strcpy(a_pt->s_fghex, r_pt->s_fghex); 
          a_pt->c_fgansi = r_pt->c_fgansi;
          if ((*(r_pt->s_bghex) || r_pt->c_bgansi) ) {
             strcpy(a_pt->s_bghex, r_pt->s_bghex); 
             a_pt->c_bgansi = r_pt->c_bgansi;
          }
          if ( i_replace ) {
             a_pt->i_special &= ~i_replace;
             a_pt->i_special |= r_pt->i_special;
          } else {
             a_pt->i_special |= r_pt->i_special;
          }
      /* Just match if ANSI bg hex : bg to bg */
      } else if ( !*(s_pt->s_fghex) &&
                  ((*(a_pt->s_bghex) && i_bg) || (*(s_pt->s_bghex) && (strcmp(s_pt->s_bghex, a_pt->s_bghex) == 0))) &&
                  !(s_pt->c_fgansi) &&
                  !(s_pt->c_bgansi) &&
                  !(i_search && (a_pt->i_special & i_search)) &&
                  !(s_pt->i_special) ) {
          strcpy(a_pt->s_bghex, r_pt->s_bghex); 
          a_pt->c_bgansi = r_pt->c_bgansi;
          if ( (*(r_pt->s_fghex) || r_pt->c_fgansi) ) {
             strcpy(a_pt->s_fghex, r_pt->s_fghex); 
             a_pt->c_fgansi = r_pt->c_fgansi;
          }
          if ( i_replace ) {
             a_pt->i_special &= ~i_replace;
             a_pt->i_special |= r_pt->i_special;
          } else {
             a_pt->i_special |= r_pt->i_special;
          }
      /* Just match if ANSI fg normal : fg to fg */
      } else if ( !*(s_pt->s_fghex) &&
                  !*(s_pt->s_bghex) &&
                  (((a_pt->c_fgansi) && i_fg) || (s_pt->c_fgansi && (s_pt->c_fgansi == a_pt->c_fgansi))) &&
                  !(s_pt->c_bgansi) &&
                  !(i_search && (a_pt->i_special & i_search)) &&
                  !(s_pt->i_special) ) {
          strcpy(a_pt->s_fghex, r_pt->s_fghex); 
          a_pt->c_fgansi = r_pt->c_fgansi;
          if ( (*(r_pt->s_bghex) || r_pt->c_bgansi) ) {
             strcpy(a_pt->s_bghex, r_pt->s_bghex); 
             a_pt->c_bgansi = r_pt->c_bgansi;
          }
          if ( i_replace ) {
             a_pt->i_special &= ~i_replace;
             a_pt->i_special |= r_pt->i_special;
          } else {
             a_pt->i_special |= r_pt->i_special;
          }
      /* Just match if ANSI bg normal : bg to bg */
      } else if ( !*(s_pt->s_fghex) &&
                  !*(s_pt->s_bghex) &&
                  !(s_pt->c_fgansi) &&
                  (((a_pt->c_bgansi) && i_bg) || (s_pt->c_bgansi && (s_pt->c_bgansi == a_pt->c_bgansi))) &&
                  !(i_search && (a_pt->i_special & i_search)) &&
                  !(s_pt->i_special) ) {
          strcpy(a_pt->s_bghex, r_pt->s_bghex); 
          a_pt->c_bgansi = r_pt->c_bgansi;
          if ( (*(r_pt->s_fghex) || r_pt->c_fgansi) ) {
             strcpy(a_pt->s_fghex, r_pt->s_fghex); 
             a_pt->c_fgansi = r_pt->c_fgansi;
          }
          if ( i_replace ) {
             a_pt->i_special &= ~i_replace;
             a_pt->i_special |= r_pt->i_special;
          } else {
             a_pt->i_special |= r_pt->i_special;
          }
      /* Match negative checks here */
      } else if ( !*(s_pt->s_fghex) &&
                  !*(s_pt->s_bghex) &&
                  !(s_pt->c_fgansi) &&
                  !(s_pt->c_bgansi) &&
                  !(i_search && (a_pt->i_special & i_search)) &&
                  (!(i_search & SPLIT_NOANSI) || ((i_search & SPLIT_NOANSI) && 
                     !((a_pt->c_fgansi) || (a_pt->c_bgansi) || 
                       *(a_pt->s_fghex) || *(a_pt->s_bghex)))) && 
                  i_search &&
                  !(s_pt->i_special) &&
                  !(s_pt->c_accent) ) {
          if ( (*(r_pt->s_fghex) || r_pt->c_fgansi) ) {
             strcpy(a_pt->s_fghex, r_pt->s_fghex); 
             a_pt->c_fgansi = r_pt->c_fgansi;
          }
          if ( (*(r_pt->s_bghex) || r_pt->c_bgansi) ) {
             strcpy(a_pt->s_bghex, r_pt->s_bghex); 
             a_pt->c_bgansi = r_pt->c_bgansi;
          }
          if ( i_replace ) {
             a_pt->i_special &= ~i_replace;
             a_pt->i_special |= r_pt->i_special;
          } else {
             a_pt->i_special |= r_pt->i_special;
          }
      /* Just match if ANSI special : special to special, even if ansi-normal */
      } else if ( !*(s_pt->s_fghex) && 
                  !*(s_pt->s_bghex) && 
                  !(s_pt->c_fgansi) && 
                  !(s_pt->c_bgansi) && 
                  (!(i_search & SPLIT_NOANSI) || ((i_search & SPLIT_NOANSI) && 
                     !((a_pt->c_fgansi) || (a_pt->c_bgansi) || 
                       *(a_pt->s_fghex) || *(a_pt->s_bghex)))) && 
                  !(i_search && (a_pt->i_special & i_search)) &&
                  ((a_pt->i_special & s_pt->i_special) == s_pt->i_special) &&
                  !(s_pt->c_accent) ) {
          if ( !s_pt->i_special )
             i_mark = 0;
          else
             i_mark = 1;
          if ( i_mark && (*(r_pt->s_fghex) || r_pt->c_fgansi) ) {
             strcpy(a_pt->s_fghex, r_pt->s_fghex); 
             a_pt->c_fgansi = r_pt->c_fgansi;
             i_mark++;
          }
          if ( i_mark && (*(r_pt->s_bghex) || r_pt->c_bgansi) ) {
             strcpy(a_pt->s_bghex, r_pt->s_bghex); 
             a_pt->c_bgansi = r_pt->c_bgansi;
             i_mark++;
          }
          if ( i_mark == 1 )
             i_mark = 0;
          if ( i_replace ) {
             a_pt->i_special &= ~i_replace;
             a_pt->i_special |= r_pt->i_special;
          } else {
             if ( !i_mark || !s_pt->i_special ) {
                if ( (!a_pt->i_special) || (a_pt->i_special & s_pt->i_special) )
                   a_pt->i_special = r_pt->i_special;
             } else
                a_pt->i_special |= r_pt->i_special;
          }
      /* Just match if ANSI accent : accent to accent -- not implemented yet */
      } else if ( s_pt->c_accent == a_pt->c_accent ) {
          a_pt->c_accent = r_pt->c_accent;
      }
      /* Else just return the value */
      s_iptr++;
      a_pt++;
   }   
}

void 
clone_ansi(char *s_input, char *s_inputptr, 
           ANSISPLIT *s_insplit, ANSISPLIT *s_insplitptr,
           char *s_output, char *s_outputptr, 
           ANSISPLIT *s_outsplit, ANSISPLIT *s_outsplitptr, int i_amount)
{
   int i_cnt;

#ifdef ZENTY_ANSI
   for (i_cnt = 0; i_cnt < i_amount; i_cnt++) {
      if ( !s_inputptr || !s_insplitptr ) 
         break;
      if ( !s_outputptr || !s_outsplitptr )
         break;
      *s_outputptr = *s_inputptr;
      strcpy(s_outsplitptr->s_bghex, s_insplitptr->s_bghex);
      strcpy(s_outsplitptr->s_fghex, s_insplitptr->s_fghex);
      s_outsplitptr->c_fgansi = s_insplitptr->c_fgansi;
      s_outsplitptr->c_bgansi = s_insplitptr->c_bgansi;
      s_outsplitptr->c_accent = s_insplitptr->c_accent;
      s_outsplitptr->i_special = s_insplitptr->i_special;
      s_outsplitptr->i_ascii8 = s_insplitptr->i_ascii8;
      s_outsplitptr->i_utf8 = s_insplitptr->i_utf8;
      s_inputptr++;
      s_insplitptr++;
      s_outputptr++;
      s_outsplitptr++;
   }
#else
   for (i_cnt = 0; i_cnt < i_amount; i_cnt++) {
      if ( !s_inputptr ) 
         break;
      safe_chr(*s_inputptr, s_output, &s_outputptr);
      s_inputptr++;
   }
#endif
}

void clone_ansisplitter_two(ANSISPLIT *a_split, ANSISPLIT *b_split, ANSISPLIT *c_split) {
   ANSISPLIT *p_ap, *p_bp, *p_cp;

   p_ap = a_split;
   p_bp = b_split;
   p_cp = c_split;

   if ( !*(p_bp->s_fghex) && !*(p_bp->s_bghex) &&
        !(p_bp->i_special) && !(p_bp->c_accent) &&
        !(p_bp->c_fgansi) && !(p_bp->c_bgansi) ) {
      strcpy(p_ap->s_fghex, p_cp->s_fghex);
      strcpy(p_ap->s_bghex, p_cp->s_bghex);
      p_ap->i_special = p_cp->i_special;
      p_ap->c_accent  = p_cp->c_accent;
      p_ap->c_fgansi  = p_cp->c_fgansi;
      p_ap->c_bgansi  = p_cp->c_bgansi;
   } else {
      strcpy(p_ap->s_fghex, p_bp->s_fghex);
      strcpy(p_ap->s_bghex, p_bp->s_bghex);
      p_ap->i_special = p_bp->i_special;
      p_ap->c_accent  = p_bp->c_accent;
      p_ap->c_fgansi  = p_bp->c_fgansi;
      p_ap->c_bgansi  = p_bp->c_bgansi;
   }
   if ( p_bp->i_ascii8 ) {
      p_ap->i_ascii8  = p_bp->i_ascii8;
   } else if (p_cp->i_ascii8) {
      p_ap->i_ascii8  = p_cp->i_ascii8;
   } else {
      p_ap->i_ascii8  = 0;
   }
   if ( p_bp->i_utf8 ) {
      p_ap->i_utf8    = p_bp->i_utf8;
   } else if (p_cp->i_utf8) {
      p_ap->i_utf8    = p_cp->i_utf8;
   } else {
      p_ap->i_utf8    = 0;
   }
}

void clone_ansisplitter(ANSISPLIT *a_split, ANSISPLIT *b_split) {
   ANSISPLIT *p_ap, *p_bp;

   p_ap = a_split;
   p_bp = b_split;

   strcpy(p_ap->s_fghex, p_bp->s_fghex);
   strcpy(p_ap->s_bghex, p_bp->s_bghex);
   p_ap->i_special = p_bp->i_special;
   p_ap->c_accent  = p_bp->c_accent;
   p_ap->c_fgansi  = p_bp->c_fgansi;
   p_ap->c_bgansi  = p_bp->c_bgansi;
   p_ap->i_ascii8  = p_bp->i_ascii8;
   p_ap->i_utf8    = p_bp->i_utf8;
}

void initialize_ansisplitter(ANSISPLIT *a_split, int i_size) {
   int i;
   ANSISPLIT *p_bp;

   p_bp = a_split;
   for ( i=0; i < i_size; i++) {
      memset(p_bp->s_fghex, '\0', 5);
      memset(p_bp->s_bghex, '\0', 5);
      p_bp->i_special = 0;
      p_bp->i_ascii8 = 0;
      p_bp->i_utf8 = 0;
      p_bp->c_accent = '\0';
      p_bp->c_fgansi = '\0';
      p_bp->c_bgansi = '\0';
      p_bp++;
   }
}

char *
rebuild_ansi(char *s_input, ANSISPLIT *s_split, int i_key) {
   char *s_buffer;
#ifdef ZENTY_ANSI
   char *s_inptr, *s_buffptr, *s_format;
   int i_ansi, i_normalize, i_normalize2;
   ANSISPLIT *s_ptr, s_last;

   memset(s_last.s_bghex, '\0', 5);
   memset(s_last.s_fghex, '\0', 5);
   s_last.c_fgansi = '\0';
   s_last.c_bgansi = '\0';
   s_last.c_accent = '\0';
   s_last.i_special = 0;
   s_last.i_ascii8 = 0;
   s_last.i_utf8 = 0;

   s_buffptr = s_buffer = alloc_lbuf("rebuild_ansi");
   s_format = alloc_sbuf("rebuild_ansi");

   s_inptr = s_input;
   s_ptr = s_split;
   i_normalize = i_normalize2 = 0;
   while ( s_inptr && *s_inptr ) {
      i_ansi = 0;
      if ( !s_ptr )
         break;

      /* Ansi changed and we should look to normalize it */
      if ( i_normalize && ((s_ptr->c_bgansi != s_last.c_bgansi) ||
                           (s_ptr->c_fgansi != s_last.c_fgansi) ||
                           strcmp(s_ptr->s_fghex, s_last.s_fghex) ||
                           strcmp(s_ptr->s_bghex, s_last.s_bghex) ||
                           (s_ptr->i_special != s_last.i_special) )) {
         i_ansi = -1;
         safe_chr('%', s_buffer, &s_buffptr);
         safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
         safe_chr('n', s_buffer, &s_buffptr);
         i_normalize = 0;
      }

      if ( i_normalize2 && (s_ptr->c_accent != s_last.c_accent) ) {
         safe_chr('%', s_buffer, &s_buffptr);
         safe_str("fn", s_buffer, &s_buffptr);
         i_normalize2 = 0;
      }
                           
      if ( s_ptr->i_special && ((i_ansi == -1) || (s_ptr->i_special != s_last.i_special)) ) {
         if ( s_ptr->i_special & SPLIT_HILITE ) {
            safe_chr('%', s_buffer, &s_buffptr);
            safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
            safe_chr('h', s_buffer, &s_buffptr);
            i_normalize = 1;
         }
         if ( s_ptr->i_special & SPLIT_FLASH ) {
            safe_chr('%', s_buffer, &s_buffptr);
            safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
            safe_chr('f', s_buffer, &s_buffptr);
            i_normalize = 1;
         }
         if ( s_ptr->i_special & SPLIT_UNDERSCORE ) {
            safe_chr('%', s_buffer, &s_buffptr);
            safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
            safe_chr('u', s_buffer, &s_buffptr);
            i_normalize = 1;
         }
         if ( s_ptr->i_special & SPLIT_INVERSE ) {
            safe_chr('%', s_buffer, &s_buffptr);
            safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
            safe_chr('i', s_buffer, &s_buffptr);
            i_normalize = 1;
         }
      }

      if ( (s_ptr->s_fghex[0] == '0') && (ToUpper(s_ptr->s_fghex[1]) == 'X') && 
           isxdigit(s_ptr->s_fghex[2]) && isxdigit(s_ptr->s_fghex[3]) && 
           ((i_ansi == -1) || strcmp(s_ptr->s_fghex, s_last.s_fghex)) ) {
         if ( i_ansi < 0 )
            i_ansi = 1;
         else
            i_ansi |= 1;
         safe_chr('%', s_buffer, &s_buffptr);
         safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
         safe_str(s_ptr->s_fghex, s_buffer, &s_buffptr);
         i_normalize = 1;
      }
      if ( (s_ptr->s_bghex[0] == '0') && (ToUpper(s_ptr->s_bghex[1]) == 'X') && 
           isxdigit(s_ptr->s_bghex[2]) && isxdigit(s_ptr->s_bghex[3]) && 
           ((i_ansi == -1) || strcmp(s_ptr->s_bghex, s_last.s_bghex)) ) {
         if ( i_ansi < 0 )
            i_ansi = 2;
         else
            i_ansi |= 2;
         safe_chr('%', s_buffer, &s_buffptr);
         safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
         safe_str(s_ptr->s_bghex, s_buffer, &s_buffptr);
         i_normalize = 1;
      }
      if ( (i_ansi != 1) && (i_ansi != 3) ) {
         if ( s_ptr->c_fgansi && ((i_ansi == -1) || (s_ptr->c_fgansi != s_last.c_fgansi)) ) {
            if ( isAnsi[(int) (s_ptr->c_fgansi)] ) {
               safe_chr('%', s_buffer, &s_buffptr);
               safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
               safe_chr(s_ptr->c_fgansi, s_buffer, &s_buffptr);
               i_normalize = 1;
            }
         }
      }
      if ( (i_ansi != 2) && (i_ansi != 3) ) {
         if ( s_ptr->c_bgansi && ((i_ansi == -1) || (s_ptr->c_bgansi != s_last.c_bgansi)) ) {
            if ( isAnsi[(int) (s_ptr->c_bgansi)] ) {
               safe_chr('%', s_buffer, &s_buffptr);
               safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
               safe_chr(s_ptr->c_bgansi, s_buffer, &s_buffptr);
               i_normalize = 1;
            }
         }
      }
      if ( s_ptr->c_accent && ((s_ptr->c_accent != s_last.c_accent)) ) {
         safe_chr('%', s_buffer, &s_buffptr);
         safe_chr('f', s_buffer, &s_buffptr);
         safe_chr(s_ptr->c_accent, s_buffer, &s_buffptr);
         i_normalize2 = 1;
      }
      if ( !s_ptr->c_accent && ((s_ptr->c_accent != s_last.c_accent)) ) {
         safe_chr('%', s_buffer, &s_buffptr);
         safe_str("fn", s_buffer, &s_buffptr);
         i_normalize2 = 0;
      }

      strcpy(s_last.s_bghex, s_ptr->s_bghex);
      strcpy(s_last.s_fghex, s_ptr->s_fghex);
      s_last.c_fgansi = s_ptr->c_fgansi;
      s_last.c_bgansi = s_ptr->c_bgansi;
      s_last.c_accent = s_ptr->c_accent;
      s_last.i_special = s_ptr->i_special;
      /* no need for s_last duplicating i_ascii8 */
      /* i_ascii8 handler.  Unicode/UTF8 will work similarly -- nudge nudge */
      if ( (*s_inptr == '?') && (s_ptr->i_utf8 > 0) ) {
        safe_chr('%', s_buffer, &s_buffptr);
        if ( s_ptr->i_utf8 > 0xfffffff ) {
           sprintf(s_format, "<u%08x>", s_ptr->i_utf8);
        } else if ( s_ptr->i_utf8 > 0xffffff ) {
           sprintf(s_format, "<u%07x>", s_ptr->i_utf8);
        } else if ( s_ptr->i_utf8 > 0xfffff ) {
           sprintf(s_format, "<u%06x>", s_ptr->i_utf8);
        } else if ( s_ptr->i_utf8 > 0xffff ) {
           sprintf(s_format, "<u%05x>", s_ptr->i_utf8);
        } else {
           sprintf(s_format, "<u%04x>", s_ptr->i_utf8);
        }
        safe_str(s_format, s_buffer, &s_buffptr);
      } else if ( (*s_inptr == '?') && (s_ptr->i_ascii8 > 0) ) {
         safe_chr('%', s_buffer, &s_buffptr);
         sprintf(s_format, "<%03d>", s_ptr->i_ascii8);
         safe_str(s_format, s_buffer, &s_buffptr);
      } else {
         safe_chr(*s_inptr, s_buffer, &s_buffptr);
      }
      s_inptr++;
      s_ptr++;
   }
   if ( i_normalize ) {
      safe_chr('%', s_buffer, &s_buffptr);
      safe_chr(SAFE_CHR, s_buffer, &s_buffptr);
      safe_chr('n', s_buffer, &s_buffptr);
   }
   if ( i_normalize2 ) {
      safe_chr('%', s_buffer, &s_buffptr);
      safe_str("fn", s_buffer, &s_buffptr);
   }
   free_sbuf(s_format);
#else
   s_buffer = alloc_lbuf("rebuild_ansi");
   strcpy(s_buffer, s_input);
#endif

   return s_buffer;
}

void
split_ansi(char *s_input, char *s_output, ANSISPLIT *s_split) {
#ifdef ZENTY_ANSI

   ANSISPLIT *s_ptr;
   char *s_inptr, *s_outptr;
   int i_hex1, i_hex2, i_ansi1, i_ansi2, i_special, i_accent, utfcnt;
   char buf_utf8[17];

   i_hex1 = i_hex2 = i_ansi1 = i_ansi2 = i_special = i_accent = 0;
   if ( !s_input || !*s_input || !s_output || !s_split ) {
      *s_output = '\0';
      return;
   }

   s_inptr = s_input;
   s_outptr = s_output;
   s_ptr = s_split;

   memset(buf_utf8, '\0', sizeof(buf_utf8));
   memset(s_ptr->s_fghex, '\0', 5);
   memset(s_ptr->s_bghex, '\0', 5);
   s_ptr->c_fgansi = '\0';
   s_ptr->c_bgansi = '\0';
   s_ptr->c_accent = '\0';
   s_ptr->i_special = 0;
   s_ptr->i_ascii8 = 0;
   s_ptr->i_utf8 = 0;
   while ( s_inptr && *s_inptr ) {
      if ( (*s_inptr == '%') && ((*(s_inptr+1) == SAFE_CHR)
#ifdef SAFE_CHR2
                        || (*(s_inptr+1) == SAFE_CHR2)
#endif
#ifdef SAFE_CHR3
                        || (*(s_inptr+1) == SAFE_CHR3)
#endif
)) {
         if ( isAnsi[(int) *(s_inptr+2)] ) {
            switch (*(s_inptr+2)) {
               case 'f':
               case 'F': s_ptr->i_special |= SPLIT_FLASH;
                         i_special = 1;
                         break;
               case 'h':
               case 'H': s_ptr->i_special |= SPLIT_HILITE;
                         i_special = 1;
                         break;
               case 'u':
               case 'U': s_ptr->i_special |= SPLIT_UNDERSCORE;
                         i_special = 1;
                         break;
               case 'i':
               case 'I': s_ptr->i_special |= SPLIT_INVERSE;
                         i_special = 1;
                         break;
               case 'N': s_ptr->i_special |= SPLIT_NOANSI;
                         i_special = 1;
                         break;
               case 'n': s_ptr->i_special = 0;
                         memset(s_ptr->s_fghex, '\0', 5);
                         memset(s_ptr->s_bghex, '\0', 5);
                         s_ptr->c_fgansi ='\0';
                         s_ptr->c_bgansi ='\0';
                         i_special = i_ansi1 = i_ansi2 = i_hex1 = i_hex2 = 0;
                         break;
               default:  if ( ToUpper(*(s_inptr+2)) == *(s_inptr+2) ) {
                            i_ansi2 = 1;
                            s_ptr->c_bgansi = *(s_inptr+2);
                            memset(s_ptr->s_bghex, '\0', 5);
                            i_hex2 = 0;
                         } else {
                            i_ansi1 = 1;
                            s_ptr->c_fgansi = *(s_inptr+2);
                            memset(s_ptr->s_fghex, '\0', 5);
                            i_hex1 = 0;
                         }
                         break;
            }
            s_inptr+=3;
            continue;
         }
         if ( (*(s_inptr+2) == '0') && ((*(s_inptr+3) == 'x') || (*(s_inptr+3) == 'X')) &&
              *(s_inptr+4) && *(s_inptr+5) && isxdigit(*(s_inptr+4)) && isxdigit(*(s_inptr+5)) ) {
            if ( *(s_inptr+3) == 'X' ) {
               i_hex2 = 1;
               i_ansi2 = 0;
               sprintf(s_ptr->s_bghex, "0%c%c%c", *(s_inptr+3), *(s_inptr+4), *(s_inptr+5));
               s_ptr->c_bgansi ='\0';
            } else {
               i_hex1 = 1;
               i_ansi1 = 0;
               sprintf(s_ptr->s_fghex, "0%c%c%c", *(s_inptr+3), *(s_inptr+4), *(s_inptr+5));
               s_ptr->c_fgansi ='\0';
            }
            s_inptr+=6;
            continue;
         }
      }
      if ( (*s_inptr == '%') && (*(s_inptr+1) == 'f') ) {
         if ( isprint(*(s_inptr+2)) ) {
            switch ( *(s_inptr+2) ) {
               case 'n':
               case 'N': s_ptr->c_accent = '\0';
                         i_accent = 0;
                         break;
               default:  s_ptr->c_accent = *(s_inptr+2);
                         i_accent = 1;
                         break;
            }
            s_inptr+=3;
            continue;
         }
      }
      if ( (*s_inptr == '%') && (*(s_inptr+1) == '<') && (*(s_inptr+2) == 'u') &&
            *(s_inptr+3) && *(s_inptr+4) && 
           ((*(s_inptr+5) == '>') || 
            (*(s_inptr+5) && *(s_inptr+6) && (*(s_inptr+7) == '>')) ||
            (*(s_inptr+5) && *(s_inptr+6) && *(s_inptr+7) && (*(s_inptr+8) == '>'))
         ) ) {
        *s_outptr = '?';
        s_inptr+=3;
        memset(buf_utf8, '\0', sizeof(buf_utf8));

        utfcnt = 0;
        while (utfcnt < 16 && *s_inptr != '>') {
            buf_utf8[utfcnt] = *s_inptr;
            utfcnt++;
            s_inptr++;
        }
                
        s_ptr->i_utf8 = strtol(buf_utf8, NULL, 16);
        s_ptr->i_ascii8 = 0;
      } else if ( (*s_inptr == '%') && (*(s_inptr+1) == '<') && isdigit(*(s_inptr+2)) &&
           isdigit(*(s_inptr+3)) && isdigit(*(s_inptr+4)) && (*(s_inptr+5) == '>') ) {
         *s_outptr = '?';
         s_ptr->i_ascii8 = atoi(s_inptr+2);
         s_ptr->i_utf8 = 0;
         s_inptr+=5;
      } else {
         *s_outptr = *s_inptr;
         s_ptr->i_ascii8 = 0;
         s_ptr->i_utf8 = 0;
      }
      s_ptr++;
      if ( !s_ptr || !s_outptr ) 
         break;
      if ( i_hex1 ) 
         strcpy(s_ptr->s_fghex, (s_ptr-1)->s_fghex);
      else
         memset((s_ptr-1)->s_fghex, '\0', 5);
      if ( i_hex2 )
         strcpy(s_ptr->s_bghex, (s_ptr-1)->s_bghex);
      else
         memset((s_ptr-1)->s_bghex, '\0', 5);
      if ( i_ansi1 )
         s_ptr->c_fgansi = (s_ptr-1)->c_fgansi;
      else
         (s_ptr-1)->c_fgansi = '\0';
      if ( i_ansi2 )
         s_ptr->c_bgansi = (s_ptr-1)->c_bgansi;
      else
         (s_ptr-1)->c_bgansi = '\0';
      if ( i_special )
         s_ptr->i_special = (s_ptr-1)->i_special;
      else
         (s_ptr-1)->i_special = 0;
      if ( i_accent )
         s_ptr->c_accent = (s_ptr-1)->c_accent;
      else
         (s_ptr-1)->c_accent = '\0';
      s_inptr++;
      s_outptr++;
   }
   *s_outptr = '\0';

#else
   strcpy(s_output, s_input);
#endif
}

int count_chars(const char *str, const char c)
{
  register int out = 0;
  register const char *p = str;
  if (p)
    while (*p != '\0')
      if (*p++ == c)
	out++;
  return out;
}

int count_extended(char *str) {
#ifdef ZENTY_ANSI
   char *s;
#endif
   int i_val;

   i_val = 0;
#ifdef ZENTY_ANSI
   s = str;
   while ( *s ) {
      if ( (*s == '%') && (*(s+1) == '<') && *(s+2) && *(s+3) && *(s+3) &&
           isdigit(*(s+2)) && isdigit(*(s+3)) && isdigit(*(s+4)) &&
           (*(s+5) == '>') ) {
         i_val += 5;
         s+=5;
      }
      s++;
   }
#endif
   return i_val;
}

/*
 * Returns an allocated, null-terminated array of strings, broken on SEP. The
 * array returned points into the original, >> modified << string. - mnp 7
 * feb 91
 */

/*
char **string2list(char *str, const char sep)
{
  int count = 0;
  char **out = NULL;
  char *end, *beg = str;
  if (str) {
    if (!(out = (char **) XMALLOC(sizeof(char *)*strlen(str),"string2list"))) {
      log_perror("ALC", "FAIL", NULL, "NO MEM in string2list()");
      return NULL;
    }
    for (;;) {
      while (*beg == sep)
	beg++;
      if (*beg == '\0')
	break;
      out[count++] = beg;
      for (end = beg; *end != '\0' && *end != sep; end++) ;
      if (*end == '\0')
	break;
      *end++ = '\0';
      beg = end;
    }
    out[count] = NULL;
  }
  if (out)
    out = (char **) realloc((char *) out, sizeof(char *) * count + 1);
  return out;
}
*/

/* returns the number of identical characters in the two strings */
int prefix_match(const char *s1, const char *s2)
{
  int count=0;

  while (*s1 && *s2 && (ToLower((int)*s1) == ToLower((int)*s2)))
    s1++, s2++, count++;
  /* If the whole string matched, count the null.  (Yes really.) */
  if (!*s1 && !*s2)
    count++;
  return count;
}

int minmatch (char *str, char *target, int min)
{
  while (*str && *target && (ToLower((int)*str) == ToLower((int)*target))) {
    str++;
    target++;
    min--;
  }
  if (*str) return 0;
  if (!*target) return 1;
  return ((min <= 0) ? 1 : 0);
}

char *strsave(const char *s)
{
  char *p;
  p = (char *)XMALLOC(sizeof(char) * (strlen(s)+1), "strsave");

  if(p)
    strcpy(p,s);
  return p;
}

char *strsavetotem(const char *s)
{
  char *p;
  p = (char *)XMALLOC(sizeof(char) * 21, "strsavetotem");

  if(p) {
    memset(p, '\0', 21);
    strncpy(p,s,20);
  }
  return p;
}



/* ---------------------------------------------------------------------------
 * safe_copy_str, safe_copy_chr - Copy buffers, watching for overflows.
 */

int safe_copy_buf(const char *src, int nLen, char *buff, char **bufc)
{
    int left = LBUF_SIZE - (*bufc - buff) - 1;
    if (left < nLen)
    {
        nLen = left;
    }
    memcpy(*bufc, src, nLen);
    *bufc += nLen;
    return nLen;
}

int safe_copy_str(char *src, char *buff, char **bufp, int max)
{
char	*tp;

	tp = *bufp;
	if (src == NULL) return 0;
	while (*src && ((tp - buff) < max))
		*tp++ = *src++;
	*tp = '\0';
/*	*(buff + max) = '\0'; */
	*bufp = tp;
	return strlen(src);
}

int safe_copy_strmax(char *src, char *buff, char **bufp, int max)
{
char	*tp;
int	left;
	tp = *bufp;
	if (src == NULL) return 0;
        left = (buff + max) - tp - strlen(src);
	if ( left < 1 )
           return 0;
	while (*src && ((tp - buff) < max))
		*tp++ = *src++;
	*tp = '\0';
	*bufp = tp;
	return strlen(src);
}

int safe_copy_chr(char src, char *buff, char **bufp, int max)
{
char	*tp;
int	retval;

	if ( src == '\032' )
           return 0;
	tp = *bufp;
	retval = 0;
	if ((tp - buff) < max) {
		*tp++ = src;
	} else {
		retval = 1;
	}
	*bufp = tp;
	*tp = '\0';
/*	*(buff + max) = '\0'; */
	return retval;
}

int matches_exit_from_list (char *str, char *pattern)
{
char	*s;

	while (*pattern) {
		for (s=str;	/* check out this one */
		     (*s && (ToLower((int)*s) == ToLower((int)*pattern)) &&
		      *pattern && (*pattern != EXIT_DELIMITER));
		     s++,pattern++) ;

		/* Did we match it all? */

		if (*s == '\0') {

			/* Make sure nothing afterwards */

			while (*pattern && isspace((int)*pattern))
				pattern++;

			/* Did we get it? */

			if (!*pattern || (*pattern == EXIT_DELIMITER))
				return 1;		  
		}

		/* We didn't get it, find next string to test */

		while (*pattern && *pattern++ != EXIT_DELIMITER) ;
		while (isspace((int)*pattern))
			pattern++;
	}
	return 0;
}

char *myitoa(int n)
{
  static char itoabuf[40];
  int i, sign;

  if ((sign = n) < 0)
    n = -n;
  itoabuf[39] = '\0';
  i = 38;
  do {
    itoabuf[i--] = n % 10 + '0';
  } while ((n /= 10) > 0);
  if (sign < 0)
    itoabuf[i] = '-';
  else
    i++;
  return (itoabuf + i);
}

/*
 * Translate string unparsing results
 */
char 
*translate_string(const char *str, int type)
{
    char old[LBUF_SIZE+1];
    static char new0[LBUF_SIZE+1];
    char *j, *c, *bp;
    int i, i_keyval;

    memset(new0, 0, sizeof(new0));
    memset(old, 0, sizeof(old));
    bp = new0;
    strncpy(old, str, LBUF_SIZE);
    i_keyval = 0;
        
    for (j = old; *j != '\0'; j++) {
        switch (*j) {
        case ESC_CHAR:
            c = strchr(j, 'm');
            if (c) {
                if (!type) {
                    j = c;
                    break;
                }
                if ( i_keyval == 0 ) {
                   safe_str("[ansi(", new0, &bp);
                   i_keyval = 1;
                }
                *c = '\0';
                i = atoi(j + 2);
                if ( (i_keyval == 3) && (i != 0) ) {
                   safe_chr(',', new0, &bp);
                   i_keyval = 1;
                }
                if ( (i_keyval == 4) && (i != 0) ) {
                   safe_chr(',', new0, &bp);
                   i_keyval = 1;
                }
                switch (i) {
                case 0:
                    if ( i_keyval == 4 )
                       safe_str(",n", new0, &bp);
                    i_keyval = ((i_keyval == 4) ? 1 : 4);
                    break;
                case 1:
                    safe_chr('h', new0, &bp);
                    break;
                case 5:
                    safe_chr('f', new0, &bp);
                    break;
                case 7:
                    safe_chr('i', new0, &bp);
                    break;
                case 30:
                    safe_chr('x', new0, &bp);
                    break;
                case 31:
                    safe_chr('r', new0, &bp);
                    break;
                case 32:
                    safe_chr('g', new0, &bp);
                    break;
                case 33:
                    safe_chr('y', new0, &bp);
                    break;
                case 34:
                    safe_chr('b', new0, &bp);
                    break;
                case 35:
                    safe_chr('m', new0, &bp);
                    break;
                case 36:
                    safe_chr('c', new0, &bp);
                    break;
                case 37:
                    safe_chr('w', new0, &bp);
                    break;
                case 40:
                    safe_chr('X', new0, &bp);
                    break;
                case 41:
                    safe_chr('R', new0, &bp);
                    break;
                case 42:
                    safe_chr('G', new0, &bp);
                    break;
                case 43:
                    safe_chr('Y', new0, &bp);
                    break;
                case 44:
                    safe_chr('B', new0, &bp);
                    break;
                case 45:
                    safe_chr('M', new0, &bp);
                    break;
                case 46:
                    safe_chr('C', new0, &bp);
                    break;
                case 47:
                    safe_chr('W', new0, &bp);
                    break;
                }
                j = c;
            } else {
                safe_chr(*j, new0, &bp);
            }
            break;
        case ' ':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if ((*(j+1) == ' ') && type)
                safe_str("%b", new0, &bp);
            else 
                safe_chr(' ', new0, &bp);
            break;
        case '\\':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("\\", new0, &bp);
            else
                safe_chr('\\', new0, &bp);
            break;
        case '%':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("%%", new0, &bp);
            else
                safe_chr('%', new0, &bp);
            break;
        case '[':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("%[", new0, &bp);
            else
                safe_chr('[', new0, &bp);
            break;
        case ']':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("%]", new0, &bp);
            else
                safe_chr(']', new0, &bp);
            break;
        case '{':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("%{", new0, &bp);
            else
                safe_chr('{', new0, &bp);
            break;
        case '}':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("%}", new0, &bp);
            else
                safe_chr('}', new0, &bp);
            break;
        case '(':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("%(", new0, &bp);
            else
                safe_chr('(', new0, &bp);
            break;
        case ')':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("%)", new0, &bp);
            else
                safe_chr(')', new0, &bp);
            break;
        case '\r':
            break;
        case '\n':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("%r", new0, &bp);
            else
                safe_chr(' ', new0, &bp);
            break;
        case '\t':
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            if (type)
                safe_str("%t", new0, &bp);
            else
                safe_chr(' ', new0, &bp);
	    break;
        default:
            if ( i_keyval == 1 ) {
               safe_chr(',', new0, &bp);
               i_keyval = 3;
            }
            if ( i_keyval == 4 ) {
               safe_str(")]", new0, &bp);
               i_keyval = 0;
            }
            safe_chr(*j, new0, &bp);
        }
    }
    if ( i_keyval != 0 ) 
       safe_str(")]", new0, &bp);
    *bp = '\0';
    return new0;
}

int tboolchk(char *s_instr)
{
    char *bp;
    bp = s_instr;

    if (!bp || !*bp) {
      return 0;
    }
    if (*bp == '#' && *(bp+1) == '-') {
      return 0;
    }
    if ( (*bp == '0') && !*(bp+1)) {
      return 0;
    }
    while (*bp == ' ' && *bp)
      bp++;
    if (*bp)
       return 1;
    else
       return 0;
}

#ifndef STANDALONE
char *
find_cluster(dbref thing, dbref player, int anum)
{
   char *s_instr, *s_instrptr, *s_text, *s_strtok, *s_strtokptr, 
        *s_useme, *s_shoveattr;
   dbref thing2, aowner;
   int aflags, i_value;
   ATTR *a_clust;
 
   s_instrptr = s_instr = alloc_lbuf("find_cluster");
   
   if ( !Good_chk(thing) || !Cluster(thing) ) {
      safe_str("#-1", s_instr, &s_instrptr);
      return(s_instr);
   }
   if ( anum > 0 ) {
      a_clust = atr_num(anum);
      if ( a_clust ) {
         anum = a_clust->number;
      } else {
         anum = -1;
      }
   } else {
      anum = -1;
   }
   a_clust = atr_str_cluster("_CLUSTER");
   if ( !a_clust ) {
      safe_str("#-1", s_instr, &s_instrptr);
      return(s_instr);
   }

   s_text = atr_get(thing, a_clust->number, &aowner, &aflags);
   if ( !*s_text ) {
      free_lbuf(s_text);
      safe_str("#-1", s_instr, &s_instrptr);
      return(s_instr);
   }

   if ( anum > 0 )
      s_shoveattr = alloc_lbuf("find_cluster_alloc");

   i_value = 2000000000;
   s_useme = NULL;

   s_strtok = strtok_r(s_text, " ", &s_strtokptr);
   while ( s_strtok ) {
      thing2 = match_thing(player, s_strtok);
      if ( Good_chk(thing2) && Cluster(thing2) ) {
         if ( i_value > db[thing2].nvattr ) {
            i_value = db[thing2].nvattr;
            s_useme = s_strtok;
         }
         if ( anum > 0 ) {
            atr_get_str(s_shoveattr, thing2, anum, &aowner, &aflags);
            if ( *s_shoveattr ) {
               s_useme = s_strtok;
               break;
            }
         }
      }
      s_strtok = strtok_r(NULL, " ", &s_strtokptr);
   }
   if ( anum > 0 )
      free_lbuf(s_shoveattr);
   if ( s_useme )
      safe_str(s_useme, s_instr, &s_instrptr);
   else
      safe_str("#-1", s_instr, &s_instrptr);

   free_lbuf(s_text);
   return(s_instr);
}

void
trigger_cluster_action(dbref thing, dbref player)
{
   char *s_tmpstr, *s_strtok, *s_strtokptr;
   int i_lowball, i_highball, aflags;
   dbref aowner, thing2;
   ATTR *attr;

   if ( !Good_chk(thing) || !Cluster(thing) )
      return;

   attr = atr_str_cluster("_CLUSTER");
   if ( !attr )
      return;

   s_tmpstr = atr_get(thing, attr->number, &aowner, &aflags);
   s_strtok = strtok_r(s_tmpstr, " ", &s_strtokptr);
   i_lowball = 2000000000;
   while ( s_strtok ) {
      thing2 = match_thing(player, s_strtok);
      if ( Good_chk(thing2) && isThing(thing2) && Cluster(thing2) ) {
         if ( (db[thing2].nvattr - 1) < i_lowball ) {
            i_lowball = (db[thing2].nvattr - 1);
         }
      }
      s_strtok = strtok_r(NULL, " ", &s_strtokptr);
   }
   free_lbuf(s_tmpstr);
   attr = atr_str_cluster("_CLUSTER_THRESH");
   i_highball = 0;
   if ( attr ) {
      s_tmpstr = atr_get(thing, attr->number, &aowner, &aflags);
      if ( *s_tmpstr ) {
         i_highball = atoi(s_tmpstr) - 1;
      }
      free_lbuf(s_tmpstr);
      if ( (i_highball > 0) && (i_highball < i_lowball) ) {
         attr = atr_str_cluster("_CLUSTER_ACTION_FUNC");
         if ( !attr ) {
            attr = atr_str_cluster("_CLUSTER_ACTION");
            if ( attr && (mudstate.clust_time + mudconf.cluster_cap) < mudstate.now ) {
               did_it(thing, thing, 0, NULL, 0, NULL, attr->number, (char **) NULL, 0);
               mudstate.clust_time = mudstate.now;
            }
         } else if ( (mudstate.clust_time + mudconf.clusterfunc_cap) < mudstate.now ) {
            s_strtok = atr_get(thing, attr->number, &aowner, &aflags);
            if ( s_strtok && *s_strtok ) {
               s_tmpstr = cpuexec(thing, thing, thing, EV_STRIP | EV_FCHECK | EV_EVAL, 
                               s_strtok, (char **)NULL, 0, (char **)NULL, 0);
               free_lbuf(s_tmpstr);
               mudstate.clust_time = mudstate.now;
            }
            free_lbuf(s_strtok);
         }
      }
   }
}

/***
 * Convert the UTF-8 bytes represented as a string
 * of hex values to a string of hex values representing
 * the Unicode code point.
 ***/
char *
encode_utf8(char *myutf) 
{
    char *ucp, *result;
    int i_ucp;
    
    result = alloc_sbuf("encode_utf8");
    memset(result, '\0', SBUF_SIZE);
    
    // Convert UTF-8 Bytes to Unicode Code Point
    ucp = utf8toucp(myutf);
    
    // Encode into parser string
    i_ucp = (int)strtol(ucp, NULL, 16);
    switch (i_ucp) {
        case DOUBLE_QUOTE_LEFT:
        case DOUBLE_QUOTE_RIGHT:
        case DOUBLE_QUOTE_REVERSED:
            if (!mudconf.allow_fancy_quotes) {
                sprintf(result, "%c", ASCII_DOUBLE_QUOTE);
            } else {
                sprintf(result, "%c<u%.20s>", '%', ucp);
            }
            break;

        case FULLWIDTH_COLON:
            if (!mudconf.allow_fullwidth_colon) {
                sprintf(result, "%c", ASCII_COLON);
            } else {
                sprintf(result, "%c<u%.20s>", '%', ucp);
            }
            break;
        case ASCII_SPACE:
            sprintf(result, "%s", (char *)" ");
            break;
        default:
            sprintf(result, "%c<u%.20s>", '%', ucp);
            break;
    }
    
    free_sbuf(ucp);
    
    return  result;
}

char *
utf8toucp(char *myutf)
{
    char *ucp, *ptr, tmp[4];
    int i_b1, i_b2, i_b3, i_b4, i_bytecnt, i_ucp;
    
/*  tmp = (char*)malloc(3); */
    ucp = alloc_sbuf("utf8toucp");
    memset(tmp, '\0', 4);
    memset(ucp, '\0', SBUF_SIZE);
    
    i_bytecnt = strlen(myutf) / 2;
    
    // Convert UTF-8 Bytes to Unicode Code Point
    if (i_bytecnt == 1) {
        return myutf;
    } else if (i_bytecnt == 2) {
        strncpy(tmp, myutf, 2);
        i_b1 = strtol(tmp, &ptr, 16);
        strncpy(tmp, myutf+2, 2);
        i_b2 = strtol(tmp, &ptr, 16);       
        i_ucp = ((i_b1 - 192) * 64) + (i_b2 - 128);
        sprintf(ucp, "%04x", i_ucp);
    } else if (i_bytecnt == 3) {
        strncpy(tmp, myutf, 2);
        i_b1 = strtol(tmp, &ptr, 16);
        strncpy(tmp, myutf+2, 2);
        i_b2 = strtol(tmp, &ptr, 16);
        strncpy(tmp, myutf+4, 2);
        i_b3 = strtol(tmp, &ptr, 16);
        i_ucp = ((i_b1 - 224) * 4096) + ((i_b2 - 128) * 64) + (i_b3 - 128);
        sprintf(ucp, "%04x", i_ucp);
    } else if (i_bytecnt == 4) {
        strncpy(tmp, myutf, 2);
        i_b1 = strtol(tmp, &ptr, 16);
        strncpy(tmp, myutf+2, 2);
        i_b2 = strtol(tmp, &ptr, 16);
        strncpy(tmp, myutf+4, 2);
        i_b3 = strtol(tmp, &ptr, 16);
        strncpy(tmp, myutf+6, 2);
        i_b4 = strtol(tmp, &ptr, 16);
        i_ucp = ((i_b1 - 240) * 262144) + ((i_b2 - 128) * 4096) + ((i_b3 - 128) * 64) + (i_b4 - 128); // Math fix. Add i_b4, don't subtract. By eery
        sprintf(ucp, "%04x", i_ucp);
    } else {
        sprintf(ucp, "0020");
    }
    
/*  free(tmp); */
    
    return  ucp;
}

/***
 * Convert string representation of Unicode code point hex values
 * to string representation of UTF8 byte hex values
 */
char *
ucptoutf8(char *ucp)
{
    char *ptr, *myutf;
    int i_ucp, i_b1, i_b2, i_b3, i_b4;
    
    myutf = alloc_sbuf("ucptoutf8");
    memset(myutf, '\0', SBUF_SIZE);
    
    i_ucp = strtol(ucp, &ptr, 16);
    
    if ( i_ucp > 31 && i_ucp <= 127) {  // Single byte, return value and not return code
        sprintf(myutf, "%02x", i_ucp);
    } else if (i_ucp <= 2047) { // 2 byte
        i_b1 = (i_ucp / 64) + 192;
        i_b2 = (i_ucp % 64) + 128;      
        sprintf(myutf, "%02x%02x", i_b1, i_b2);
    } else if (i_ucp <= 65535) { // 3 byte
        i_b1 = (i_ucp / 4096) + 224;
        i_b2 = ((i_ucp % 4096) / 64) + 128;
        i_b3 = (i_ucp % 64) + 128;
        sprintf(myutf, "%02x%02x%02x", i_b1, i_b2, i_b3);
    } else if (i_ucp <= 1114111) { // 4 byte
        i_b1 = (i_ucp / 262144) + 240;
        i_b2 = ((i_ucp % 262144) / 4096) + 128;
        i_b3 = ((i_ucp % 4096) / 64) + 128;
        i_b4 = (i_ucp % 64) + 128;
        sprintf(myutf, "%02x%02x%02x%02x", i_b1, i_b2, i_b3, i_b4);
    } else { // Invalid, return space
        sprintf(myutf, " ");
    }
    
    return myutf;
}

char ucs32toascii(long ucs)
{
   // Original code by Polk
   // Modified to a linear array search for a speedy lookup --Amb
   int i;
   long utfcodes[] = { 0x2012, '-', 0x2013, '-', 0x2014, '-',
                       0x2500, '-', 0x2501, '-', 0x2502, '|', 0x2503, '|' 
                     , 0x2504, '-', 0x2505, '-', 0x2506, '|', 0x2507, '|'
                     , 0x2508, '-', 0x2509, '-', 0x250A, '|', 0x250B, '|'
                     , 0x250C, '+', 0x250D, '+', 0x250E, '+', 0x250F, '+'
                     , 0x2510, '+', 0x2511, '+', 0x2512, '+', 0x2513, '+'
                     , 0x2514, '+', 0x2515, '+', 0x2516, '+', 0x2517, '+'
                     , 0x2518, '+', 0x2519, '+', 0x251A, '+', 0x251B, '+'
                     , 0x251C, '+', 0x251D, '+', 0x251E, '+', 0x251F, '+'
                     , 0x2520, '+', 0x2521, '+', 0x2522, '+', 0x2523, '+'
                     , 0x2524, '+', 0x2525, '+', 0x2526, '+', 0x2527, '+'
                     , 0x2528, '+', 0x2529, '+', 0x252A, '+', 0x252B, '+'
                     , 0x252C, '+', 0x252D, '+', 0x252E, '+', 0x252F, '+'
                     , 0x2530, '+', 0x2531, '+', 0x2532, '+', 0x2533, '+'
                     , 0x2534, '+', 0x2535, '+', 0x2536, '+', 0x2537, '+'
                     , 0x2538, '+', 0x2539, '+', 0x253A, '+', 0x253B, '+'
                     , 0x253C, '+', 0x253D, '+', 0x253E, '+', 0x253F, '+'
                     , 0x2540, '+', 0x2541, '+', 0x2542, '+', 0x2543, '+'
                     , 0x2544, '+', 0x2545, '+', 0x2546, '+', 0x2547, '+'
                     , 0x2548, '+', 0x2549, '+', 0x254A, '+', 0x254B, '+'
                     , 0x254C, '-', 0x254D, '-', 0x254E, '|', 0x254F, '|'
                     , 0x2550, '=', 0x2551, '|', 0x2552, '+', 0x2553, '+'
                     , 0x2554, '+', 0x2555, '+', 0x2556, '+', 0x2557, '+'
                     , 0x2558, '+', 0x2559, '+', 0x255A, '+', 0x255B, '+'
                     , 0x255C, '+', 0x255D, '+', 0x255E, '+', 0x255F, '+'
                     , 0x2560, '+', 0x2561, '+', 0x2562, '+', 0x2563, '+'
                     , 0x2564, '+', 0x2565, '+', 0x2566, '+', 0x2567, '+'
                     , 0x2568, '+', 0x2569, '+', 0x256A, '+', 0x256B, '+'
                     , 0x256C, '+', 0x256D, '+', 0x256E, '+', 0x256F, '+'
                     , 0x2570, '+', 0x2571, '/', 0x2572, '\\', 0x2573, 'X'
                     , 0x2574, '-', 0x2575, '|', 0x2576, '-', 0x2577, '|'
                     , 0x2578, '-', 0x2579, '|', 0x257A, '-', 0x257B, '|'
                     , 0x257C, '-', 0x257D, '|', 0x257E, '-', 0x257F, '|'
                     , 0x25CF, '*', 0x25CC, 'o', 0x25CB, 'o'
                     , 0x0060, '`', 0x201c, '"', 0x2019, '\'', 0x00a1, '!'
                     , 0x2022, '.', 0x2039, '<', 0x203a, '>', 0x00b0, 'o'
                     , 0x00ba, 'o', 0x00b7, '.', 0x201a, ',', 0x2550, '='
                     , 0x00ab, '<', 0x00bb, '>', 0x00c0, 'A', 0x00c1, 'A'
                     , 0x00c2, 'A', 0x00c3, 'A', 0x00c4, 'A', 0x00c5, 'A'
                     , 0x00c7, 'C', 0x00c8, 'E', 0x00c9, 'E', 0x00ca, 'E'
                     , 0x00cb, 'E', 0x00cc, 'I', 0x00cd, 'I', 0x00ce, 'I'
                     , 0x00cf, 'I', 0x00d0, 'D', 0x00d1, 'N', 0x00d2, 'O'
                     , 0x00d3, 'O', 0x00d4, 'O', 0x00d5, 'O', 0x00d6, 'O'
                     , 0x00d8, 'O', 0x00d7, 'x', 0x00d9, 'U', 0x00da, 'U'
                     , 0x00db, 'U', 0x00dc, 'U', 0x00dd, 'Y', 0x00e0, 'a'
                     , 0x00e1, 'a', 0x00e2, 'a', 0x00e3, 'a', 0x00e4, 'a'
                     , 0x00e5, 'a', 0x00e7, 'c', 0x00e8, 'e', 0x00e9, 'e'
                     , 0x00ea, 'e', 0x00eb, 'e', 0x00ec, 'i', 0x00ed, 'i'
                     , 0x00ee, 'i', 0x00ef, 'i', 0x00f0, 'd', 0x00f1, 'n'
                     , 0x00f2, 'o', 0x00f3, 'o', 0x00f4, 'o', 0x00f5, 'o'
                     , 0x00f6, 'o', 0x00f8, 'o', 0x00f7, '-', 0x00f9, 'u'
                     , 0x00fa, 'u', 0x00fb, 'u', 0x00fc, 'u', 0x00fd, 'y'
                     , 0x00bf, '?', 0x00a6, '|', 0x2030, '%', 0x0160, 'S'
                     , 0x0161, 's', 0x00df, 's' /* Sorry Ambrosia */
                     };

   /* ASCII is as ASCII does */
   if(ucs < 128)
       return ucs;

   /* Walk the substitutes array */
   for(i=0;i<sizeof(utfcodes)/sizeof(long);i+=2)
      if(utfcodes[i] == ucs)
         return utfcodes[i+1];

   /* Fallback */
   return '?';
}
#endif
