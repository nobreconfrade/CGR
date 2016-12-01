#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>

#include "maze.h"

void cut_name(char *name) {
  char *str;

  /* reduce string to 8 chars */
  name[8]=0;

  /* try to cut at space */
  if((str = strchr(name, ' '))!=NULL)
    *str=0;
}

char *get_player_name(void) {
  struct passwd *pass;
  char *comma, *cp1, *cp2;
  static char fullname[80];

  /* get environment entry */
  if((cp1=getenv("USERNAME"))!=NULL) {
    if(strlen(cp1)!=0) {
      strcpy(fullname, cp1);
      cut_name(fullname);
      return fullname;
    }
  }
  
  /* Get user information from password file */
  if (!(pass = getpwuid(getuid())))
    return(NULL);    /* unknown user */

  /* find a comma indicating further info after name */
  comma = strchr(pass->pw_gecos, ',');
  
  /* NULL out the comma */
  if (comma) *comma = '\0';
  
  /* Use the nickname if not null otherwise password file name */
  cp1 = pass->pw_gecos;
  cp2 = fullname;
  
  /* Search through the gecos field looking for an '&' which on very
   * old UNIX systems is supposed to be the users user name with the
   * first letter uppercased.
   */
  while(*cp1)
    {
      /* Look for the '&' symbol */
      if(*cp1 != '&')
	*cp2++ = *cp1++;
      else {
	/* A ha. Now copy the users name to be in place of '&' */
	strcpy(cp2, pass->pw_name);
	
	/* Convert the first letter to uppercase. */
	if(islower(*cp2))
	  *cp2 = toupper(*cp2);
	
	/* Continue with the remaining string */
	while(*cp2) cp2++;
	cp1++;
      }
    }
  
  /* Return their name without any trailing stuff */
  cut_name(fullname);
  return(fullname);
}
