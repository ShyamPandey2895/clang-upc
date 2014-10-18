/*===--- gupcr_fabric.c - UPC Runtime Support Library --------------------===
|*
|*                     The LLVM Compiler Infrastructure
|*
|* Copyright 2012-2014, Intel Corporation.  All rights reserved.
|* This file is distributed under a BSD-style Open Source License.
|* See LICENSE-INTEL.TXT for details.
|*
|*===---------------------------------------------------------------------===*/

/**
 * @file gupcr_fabric.c
 * GUPC Llibfabric Initialization.
 */

#include "gupcr_config.h"
#include "gupcr_defs.h"
#include "gupcr_utils.h"
#include "gupcr_fabric.h"
#include "gupcr_runtime.h"

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

/**
 * @addtogroup FABRIC_RUNTIME GUPCR Libfabric runtime interface
 * @{
 */

int gupcr_rank;
int gupcr_rank_cnt;
int gupcr_child[GUPCR_TREE_FANOUT];
int gupcr_child_cnt;
int gupcr_parent_thread;
struct fid_ep *gupcr_ep = NULL;

size_t gupcr_max_ordered_size;
size_t gupcr_max_msg_size;
size_t gupcr_max_optim_size;

static char *fi_src_addr;
static fab_info_t fi_hints;
static fab_t gupcr_fab;
fab_info_t gupcr_fi;
fab_domain_t gupcr_fd;

// TODO: Hack for getting network address.  We can get this from the
//       fi_getinfo (?) but current implementation does not fill out src_addr
/** Infiniband interface.  */
const char *ifname = "ib0";
/** Interface IPv4 address */
in_addr_t net_addr;
/** IPv4 addresses for all ranks */
in_addr_t *net_addr_map;

/**
 * Get IPv4 address of the specified interface
 */
in_addr_t
check_ip_address (const char *ifname)
{
  int fd;
  struct ifreq devinfo;
  struct sockaddr_in *sin = (struct sockaddr_in *)&devinfo.ifr_addr;
  in_addr_t addr;

  fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (fd < 0)
    {
      gupcr_fatal_error ("error initializing socket");
      gupcr_abort ();
    }

  strncpy(devinfo.ifr_name, ifname, IFNAMSIZ);

  if (ioctl(fd, SIOCGIFADDR, &devinfo) == 0)
    {
      addr = sin->sin_addr.s_addr;
      gupcr_log (FC_FABRIC, "IB IPv4 address for %s is %d",
	         devinfo.ifr_name, addr);
    }
  else
    addr = INADDR_ANY;
  close(fd);
  return addr;
}

/**
 * Return Libfabric error description string.
 *
 * @param [in] errnum Libfabric error number
 * @retval Error description string
 */
const char *
gupcr_strfaberror (int errnum)
{
  static char gupcr_strfaberror_buf[64];
  switch (errnum)
    {
    case 0:
      return "fabric operation successful";
    default:
      break;
    }
  sprintf (gupcr_strfaberror_buf, "unknown fabric status code: %d", errnum);
  return gupcr_strfaberror_buf;
}

/**
 * Return Event Queue type description string.
 *
 * @param [in] eqtype Event queue type
 * @retval Event queue type description string
 */
const char *
gupcr_streqtype (uint64_t eqtype)
{
  switch (eqtype)
    {
      default:
	break;
    }
  return "UNKNOWN EVENT TYPE";
}

/**
 * Return Data type description string.
 
 * @param [in] datatype Data type
 * @retval Data type description string
 */
const char *
gupcr_strdatatype (enum fi_datatype datatype)
{
  switch (datatype)
    {
    case FI_INT8:
      return "FI_INT8";
    case FI_UINT8:
      return "FI_UINT8";
    case FI_INT16:
      return "FI_INT16";
    case FI_UINT16:
      return "FI_UINT16";
    case FI_INT32:
      return "FI_INT32";
    case FI_UINT32:
      return "FI_UINT32";
    case FI_FLOAT:
      return "FI_FLOAT";
    case FI_INT64:
      return "FI_INT64";
    case FI_UINT64:
      return "FI_UINT64";
    case FI_DOUBLE:
      return "FI_DOUBLE";
    case FI_FLOAT_COMPLEX:
      return "FI_FLOAT_COMPLEX";
    case FI_DOUBLE_COMPLEX:
      return "FI_DOUBLE_COMPLEX";
    case FI_LONG_DOUBLE:
      return "FI_LONG_DOUBLE";
    case FI_LONG_DOUBLE_COMPLEX:
      return "FI_LONG_DOUBLE_COMPLEX";
    default:
      return "UNKNOWN DATA TYPE";
    }
}

/**
 * Return Atomic operation description string.
 *
 * @param [in] op Atomic operation type
 * @retval Atomic operation description string
 */
const char *
gupcr_strop (enum fi_op op)
{
  switch (op)
    {
    case FI_MIN:
      return "FI_MIN";
    case FI_MAX:
      return "FI_MAX";
    case FI_SUM:
      return "FI_SUM";
    case FI_PROD:
      return "FI_PROD";
    case FI_LOR:
      return "FI_LOR";
    case FI_LAND:
      return "FI_LAND";
    case FI_BOR:
      return "FI_BOR";
    case FI_BAND:
      return "FI_BAND";
    case FI_LXOR:
      return "FI_LXOR";
    case FI_BXOR:
      return "FI_BXOR";
    case FI_ATOMIC_READ:
      return "FI_ATOMIC_READ";
    case FI_ATOMIC_WRITE:
      return "FI_ATOMIC_WRITE";
    case FI_CSWAP:
      return "FI_CSWAP";
    case FI_CSWAP_NE:
      return "FI_CSWAP_NE";
    case FI_CSWAP_LE:
      return "FI_CSWAP_LE";
    case FI_CSWAP_LT:
      return "FI_CSWAP_LT";
    case FI_CSWAP_GE:
      return "FI_CSWAP_GE";
    case FI_CSWAP_GT:
      return "FI_CSWAP_GT";
    case FI_MSWAP:
      return "FI_MSWAP";
    default:
      return "UNKNOWN ATOMIC OPERATION TYPE";
    }
}

/**
 * Return Network Interface error description string.
 *
 * @param [in] nitype NI failure type
 * @retval NI failure description string.
 */
const char *
gupcr_nifailtype (int nitype)
{
  switch (nitype)
    {
      default:
        ;
    }
  return "NI_FAILURE_UNKNOWN";
}

/**
 * Return atomic data type from the specified size.
 */
enum fi_datatype
gupcr_get_atomic_datatype (int size)
{
  switch (size)
    {
    case 1:
      return FI_UINT8;
    case 2:
      return FI_UINT16;
    case 4:
      return FI_UINT32;
    case 8:
      return FI_UINT64;
    case 16:
      return FI_DOUBLE_COMPLEX;
    default:
      gupcr_fatal_error
	("Unable to convert size of %d into atomic data type.", size);
    }
  return -1;
}

/**
 * Return data size from the specified atomic type.
 *
 * @param [in] type atomic data type
 * @retval atomic data type size
 */
size_t
gupcr_get_atomic_size (enum fi_datatype type)
{
  switch (type)
    {
    case FI_INT8:
    case FI_UINT8:
      return 1;
    case FI_INT16:
    case FI_UINT16:
      return 2;
    case FI_INT32:
    case FI_UINT32:
      return 4;
    case FI_INT64:
    case FI_UINT64:
      return 8;
    case FI_FLOAT:
      return __SIZEOF_FLOAT__;
    case FI_FLOAT_COMPLEX:
      return 2 * __SIZEOF_FLOAT__;
    case FI_DOUBLE:
      return __SIZEOF_DOUBLE__;
    case FI_DOUBLE_COMPLEX:
      return 2 * __SIZEOF_DOUBLE__;
#ifdef __SIZEOF_LONG_DOUBLE__
    case FI_LONG_DOUBLE:
      return __SIZEOF_LONG_DOUBLE__;
    case FI_LONG_DOUBLE_COMPLEX:
      return 2 * __SIZEOF_LONG_DOUBLE__;
#endif
    default:
      gupcr_fatal_error ("unknown atomic type %d", (int) type);
    }
  return -1;
}

/**
 * @fn gupcr_process_fail_events (struct fid_eq eq)
 * Show information on failed events.
 *
 * This procedure prints the contents of the event queue.  As
 * completion events are not reported, the event queue contains
 * only failure events.  This procedure is called only if any of the
 * counting events reported a failure.
 *
 * @param [in] eq Event Queue ID
 */
void
gupcr_process_fail_events (fab_eq_t eq)
{
  int ret;
  struct fi_eq_comp_entry eq_entry;
  gupcr_fabric_call_nc (fi_eq_read, ret, (eq, (void *)&eq_entry,
					  sizeof (eq_entry)));
  if (ret < 0)
    {
      char buf[256];
      const char *errstr;
      struct fi_eq_err_entry eq_error;
      gupcr_fabric_call_nc (fi_eq_readerr, ret,
			    (eq, (void *)&eq_error, sizeof (eq_error), 0));
      gupcr_fabric_call_nc (fi_eq_strerror, errstr,
			    (eq, eq_error.prov_errno, eq_error.prov_data,
			     buf, sizeof (buf)));
      gupcr_error ("%s", buf);
    }
  else
    gupcr_fatal_error ("ctr reported an error, but eq has none");
}

/**
 * Get current thread rank.
 * @retval Rank of the current thread
 */
int
gupcr_get_rank (void)
{
  return gupcr_rank;
}

/**
 * Get number of running threads.
 * @retval Number of running threads
 */
int
gupcr_get_threads_count (void)
{
  return gupcr_rank_cnt;
}

/**
 * Get process PID for specified rank.
 * @param [in] rank Rank of the thread
 * @retval PID of the thread
 */
int
gupcr_get_rank_pid (int rank)
{
  return rank;
}

/**
 * Get process NID for specified rank.
 * @param [in] rank Rank of the thread
 * @retval NID of the thread
 */
int
gupcr_get_rank_nid (int rank)
{
  return (int)net_addr_map[rank];
}

/**
 * Wait for all threads to complete initialization.
 */
void
gupcr_startup_barrier (void)
{
  gupcr_runtime_barrier ();
}

/** @} */

/**
 * @addtogroup INIT GUPCR Initialization
 * @{
 */

/**
 * Initialize Libfabric.
 */
void
gupcr_fabric_init (void)
{
  struct fi_info hints = {0};

  /* TODO - fix for new version of libfabric  */
  /* Find fabric provider based on the hints.  */
  hints.type = FID_RDM;	/* Reliable datagram message.  */
  hints.protocol = FI_PROTO_UNSPEC;
  /* Endpoint capabilities.  */
  hints.ep_cap = FI_RMA |	   /* Request RMA capability,  */
		 FI_ATOMICS |	   /* atomics capability,  */
		 FI_REMOTE_WRITE | /* inbound remote writes,  */
		 FI_REMOTE_READ;   /* inbound remote reads.  */
  hints.domain_cap = FI_DYNAMIC_MR;     /* Allow for registration of memory regions  */
					/* without physical backing.  */
  hints.op_flags = FI_EVENT |	        /* Generate completion entries,  */
		   FI_REMOTE_COMPLETE ; /* on the remote side.  */
  hints.addr_format = FI_ADDR_INDEX; /* Use index into address vector.  */
  hints.src_addrlen = sizeof(struct fi_info_addr);

  /*  '8' hardcoded for Portals PTE.  */
  gupcr_fabric_call (fi_getinfo, (NULL, "8", FI_SOURCE, &hints, &gupcr_fi));
  gupcr_fabric_call (fi_fabric, (gupcr_fi->fabric_name, 0, &gupcr_fab, NULL));
  gupcr_fabric_call (fi_fdomain, (gupcr_fab, gupcr_fi, &gupcr_fd, NULL));

  gupcr_rank = gupcr_runtime_get_rank ();
  gupcr_rank_cnt = gupcr_runtime_get_size ();

  /* TODO: Sat various fabric properties:
     gupcr_max_message_size
     gupcr_max_orderd_size
     gupcr_max_optim_size  (inject calls)

     The new interface has this parameters as part of the fabric info.
     The old one has it as part of the endpoint properties.
  */
}

/**
 * Close Libfabric.
 */
void
gupcr_fabric_fini (void)
{
  gupcr_fabric_call (fi_close, (&gupcr_fab->fid));
}

/**
 * Initialize Networking Interface.
 */
void
gupcr_fabric_ni_init (void)
{
  int status;
  /* Find the network id.  */
  net_addr = check_ip_address (ifname);
  if (net_addr == INADDR_ANY)
    {
      gupcr_fatal_error ("IPv4 is not available on interface %s", ifname);
      gupcr_abort ();
    }
  /* Allocate space for network addresses.  */
  net_addr_map = calloc (THREADS * sizeof (in_addr_t), 1);
  if (!net_addr_map)
    gupcr_fatal_error ("cannot allocate memory for net node map");
  /* Exchange network id with other nodes.  */
  status = gupcr_runtime_exchange ("NI", &net_addr, sizeof (in_addr_t),
				   net_addr_map);

}

/**
 * Close Fabric Networking Interface.
 */
void
gupcr_fabric_ni_fini (void)
{
}

/**
 * Find the node's parent and all its children.
 */
void
gupcr_nodetree_setup (void)
{
  int i;
  gupcr_log ((FC_BARRIER | FC_BROADCAST),
	     "node tree initialized with fanout of %d", GUPCR_TREE_FANOUT);
  for (i = 0; i < GUPCR_TREE_FANOUT; i++)
    {
      int child = GUPCR_TREE_FANOUT * MYTHREAD + i + 1;
      if (child < THREADS)
	{
	  gupcr_child_cnt++;
	  gupcr_child[i] = child;
	}
    }
  if (MYTHREAD == 0)
    gupcr_parent_thread = ROOT_PARENT;
  else
    gupcr_parent_thread = (MYTHREAD - 1) / GUPCR_TREE_FANOUT;
}

/** @} */
