/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.scalar.conf 11805 2005-01-07 09:37:18Z dts12 $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitrate.h"
#include "iwlib.h"
#include "util.h"

/**************************** CONSTANTS ****************************/

/*
 * Error codes defined for setting args
 */
#define IWERR_ARG_NUM		-2
#define IWERR_ARG_TYPE		-3
#define IWERR_ARG_SIZE		-4
#define IWERR_ARG_CONFLICT	-5
#define IWERR_SET_EXT		-6
#define IWERR_GET_EXT		-7

/**************************** VARIABLES ****************************/

/*
 * Ugly, but deal with errors in set_info() efficiently...
 */
static int	errarg;
static int	errmax;


/***************************** BITRATES *****************************/

static int
get_bitrate(int	skfd, char *ifname)
{
  struct iwreq		wrq;
  struct iw_range	range;

  /* Extract range info */
  if(iw_get_range_info(skfd, ifname, &range) < 0) {
  	return -1;
  }
  else
    {
      /* Get current bit rate */
      if(iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) >= 0)
	{
	  return wrq.u.bitrate.value;
	}

    }
  return -2;
}

/*------------------------------------------------------------------*/
/*
 * Set Bit Rate
 */
static int
set_bitrate(int		skfd,
		 char *		ifname,
		 char *		args[],		/* Command line args */
		 int		count)		/* Args count */
{
  struct iwreq		wrq;
  int			i = 1;

  wrq.u.bitrate.flags = 0;
  if(!strcasecmp(args[0], "auto"))
    {
      wrq.u.bitrate.value = -1;
      wrq.u.bitrate.fixed = 0;
    }
  else
    {
      if(!strcasecmp(args[0], "fixed"))
	{
	  /* Get old bitrate */
	  if(iw_get_ext(skfd, ifname, SIOCGIWRATE, &wrq) < 0)
	    return(IWERR_GET_EXT);
	  wrq.u.bitrate.fixed = 1;
	}
      else			/* Should be a numeric value */
	{
	  double		brate;
	  char *		unit;

	  brate = strtod(args[0], &unit);
	  if(unit == args[0])
	    {
	      errarg = 0;
	      return(IWERR_ARG_TYPE);
	    }
	  if(unit != NULL)
	    {
	      if(unit[0] == 'G') brate *= GIGA;
	      if(unit[0] == 'M') brate *= MEGA;
	      if(unit[0] == 'k') brate *= KILO;
	    }
	  wrq.u.bitrate.value = (long) brate;
	  wrq.u.bitrate.fixed = 1;

	  /* Check for an additional argument */
	  if((i < count) && (!strcasecmp(args[i], "auto")))
	    {
	      wrq.u.bitrate.fixed = 0;
	      ++i;
	    }
	  if((i < count) && (!strcasecmp(args[i], "fixed")))
	    {
	      wrq.u.bitrate.fixed = 1;
	      ++i;
	    }
	  if((i < count) && (!strcasecmp(args[i], "unicast")))
	    {
	      wrq.u.bitrate.flags |= IW_BITRATE_UNICAST;
	      ++i;
	    }
	  if((i < count) && (!strcasecmp(args[i], "broadcast")))
	    {
	      wrq.u.bitrate.flags |= IW_BITRATE_BROADCAST;
	      ++i;
	    }
	}
    }

  if(iw_set_ext(skfd, ifname, SIOCSIWRATE, &wrq) < 0)
    return(IWERR_SET_EXT);

  /* Var args */
  return(i);
}

/** Initializes the bitrate module */
void
init_bitrate(void)
{
    static oid bitrate_oid[] = { 1,3,6,1,3,108,0,1 };

  DEBUGMSGTL(("bitrate", "Initializing\n"));

    netsnmp_register_scalar(
        netsnmp_create_handler_registration("bitrate", handle_bitrate,
                               bitrate_oid, OID_LENGTH(bitrate_oid),
                               HANDLER_CAN_RWRITE
        ));
}

int
handle_bitrate(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info   *reqinfo,
                          netsnmp_request_info         *requests)
{
    int ret;
    char bitrate_str[255];
    int bitrate; 
    static int skfd;
    /* We are never called for a GETNEXT if it's registered as a
       "instance", as it's "magically" handled for us.  */

    /* a instance handler also only hands us one request at a time, so
       we don't need to loop over a list of requests; we'll only get one. */
    
    switch(reqinfo->mode) {

        case MODE_GET:
                printf("bitrate MODE_GET\n");
	    if((skfd = iw_sockets_open()) < 0) {
	    	bitrate = -4;
	    }
	    else {
	        bitrate = get_bitrate(skfd, INTERFACE);
	    }
	    sprintf(bitrate_str, "%d", bitrate);
            snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
                                     (u_char *) bitrate_str, strlen(bitrate_str));
	    if(skfd >= 0)
		    iw_sockets_close(skfd);
            break;

        /*
         * SET REQUEST
         *
         * multiple states in the transaction.  See:
         * http://www.net-snmp.org/tutorial-5/toolkit/mib_module/set-actions.jpg
         */
        case MODE_SET_RESERVE1:
                printf("bitrate MODE_SET_RESERVE1\n");
                /* or you could use netsnmp_check_vb_type_and_size instead */
            ret = netsnmp_check_vb_type(requests->requestvb, ASN_OCTET_STR);
            if ( ret != SNMP_ERR_NOERROR ) {
                netsnmp_set_request_error(reqinfo, requests, ret );
            }
            break;

        case MODE_SET_RESERVE2:
                printf("bitrate MODE_SET_RESERVE2\n");
            /* malloc "undo" storage buffer */
	    if((skfd = iw_sockets_open()) < 0) {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_RESOURCEUNAVAILABLE);
            }
            break;

        case MODE_SET_FREE:
                printf("bitrate MODE_SET_FREE\n");
            /* free resources allocated in RESERVE1 and/or
               RESERVE2.  Something failed somewhere, and the states
               below won't be called. */
	    if(skfd >= 0)
		    iw_sockets_close(skfd);
            break;

        case MODE_SET_ACTION:
                printf("bitrate MODE_SET_ACTION\n");
            /* perform the value change here */
	    ret = set_bitrate(skfd, INTERFACE, (char**)&(requests->requestvb->val.string), 1);
            if (ret <= 0) {
                netsnmp_set_request_error(reqinfo, requests, ret);
            }
            break;

        case MODE_SET_COMMIT:
                printf("bitrate MODE_SET_COMMIT\n");
            /* delete temporary storage */
	    if(skfd >= 0)
		    iw_sockets_close(skfd);
            break;

        case MODE_SET_UNDO:
                printf("bitrate MODE_SET_UNDO\n");
            break;

        default:
            /* we should never get here, so this is a really bad error */
            snmp_log(LOG_ERR, "unknown mode (%d) in handle_bitrate\n", reqinfo->mode );
            return SNMP_ERR_GENERR;
    }

                printf("bitrate - return\n");
    return SNMP_ERR_NOERROR;
}
