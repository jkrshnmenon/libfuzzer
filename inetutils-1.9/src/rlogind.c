/*
  Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
  2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Free
  Software Foundation, Inc.

  This file is part of GNU Inetutils.

  GNU Inetutils is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at
  your option) any later version.

  GNU Inetutils is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see `http://www.gnu.org/licenses/'. */

#include <config.h>

#include <signal.h>
#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif
#include <termios.h>
#include <sys/time.h>
#include <time.h>
#ifdef HAVE_SYS_STREAM_H
# include <sys/stream.h>
#endif
#ifdef HAVE_SYS_TTY_H
# include <sys/tty.h>
#endif
#ifdef HAVE_SYS_PTYVAR_H
# include <sys/ptyvar.h>
#endif

#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef HAVE_NETINET_IN_SYSTM_H
# include <netinet/in_systm.h>
#endif
#ifdef HAVE_NETINET_IP_H
# include <netinet/ip.h>
#endif
#include <arpa/inet.h>
#include <netdb.h>
#ifdef HAVE_UTMP_H
# include <utmp.h>
#endif

#include <pwd.h>
#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/stat.h>		/* Needed for chmod() */

#include <pty.h>

#include <progname.h>
#include <argp.h>
#include <libinetutils.h>
#include "unused-parameter.h"

/*
  The TIOCPKT_* macros may not be implemented in the pty driver.
  Defining them here allows the program to be compiled.  */
#ifndef TIOCPKT
# define TIOCPKT                 _IOW('t', 112, int)
# define TIOCPKT_FLUSHWRITE      0x02
# define TIOCPKT_NOSTOP          0x10
# define TIOCPKT_DOSTOP          0x20
#endif /*TIOCPKT*/
#ifndef TIOCPKT_WINDOW
# define TIOCPKT_WINDOW 0x80
#endif
/* `defaults' for tty settings.  */
#ifndef TTYDEF_IFLAG
# define TTYDEF_IFLAG	(BRKINT | ISTRIP | ICRNL | IMAXBEL | IXON | IXANY)
#endif
#ifndef TTYDEF_OFLAG
# ifndef OXTABS
#  define OXTABS 0
# endif
# define TTYDEF_OFLAG	(OPOST | ONLCR | OXTABS)
#endif
#ifndef TTYDEF_LFLAG
# define TTYDEF_LFLAG	(ECHO | ICANON | ISIG | IEXTEN | ECHOE|ECHOKE|ECHOCTL)
#endif
#define AUTH_KERBEROS_SHISHI 1
#define AUTH_KERBEROS_4 4
#define AUTH_KERBEROS_5 5
#if defined KERBEROS || defined SHISHI
# ifdef	KRB4
#  define SECURE_MESSAGE "This rlogin session is using DES encryption for all transmissions.\r\n"
#  include <kerberosIV/des.h>
#  include <kerberosIV/krb.h>
#  define kerberos_error_string(c) krb_err_txt[c]
#  define AUTH_KERBEROS_DEFAULT AUTH_KERBEROS_4
# elif defined(KRB5)
#  define SECURE_MESSAGE "This rlogin session is using DES encryption for all transmissions.\r\n"
#  include <krb5.h>
#  include <kerberosIV/krb.h>
#  define kerberos_error_string(c) error_message (c)
#  define AUTH_KERBEROS_DEFAULT AUTH_KERBEROS_5
# elif defined(SHISHI)
#  define SECURE_MESSAGE "This rlogin session is using encryption for all transmissions.\r\n"
#  include <shishi.h>
#  include "shishi_def.h"
#  define AUTH_KERBEROS_DEFAULT AUTH_KERBEROS_SHISHI
#  define kerberos_error_string(c) shishi_strerror(c)
# endif
#endif /* KERBEROS */
#define ENVSIZE	(sizeof("TERM=")-1)	/* skip null for concatenation */
#ifndef DEFMAXCHILDREN
# define DEFMAXCHILDREN 10	/* Default maximum number of children */
#endif
#ifndef DEFPORT
# define DEFPORT 513
#endif

#ifdef HAVE___CHECK_RHOSTS_FILE
extern int __check_rhosts_file;
#endif

#ifndef SHISHI
struct auth_data
{
  struct sockaddr_in from;
  char *hostname;
  char *lusername;
  char *rusername;
  char *term;
  char *env[2];
# ifdef KERBEROS
#  ifdef KRB5
  int kerberos_version;
  krb5_principal client;
  krb5_context context;
  krb5_ccache ccache;
  krb5_keytab keytab;
#  endif
# endif
};
#endif

#define MODE_INETD 0
#define MODE_DAEMON 1
int mode = MODE_INETD;

int port = 0;
int maxchildren = DEFMAXCHILDREN;
int allow_root = 0;
int verify_hostname = 0;
int keepalive = 1;

#if defined KERBEROS || defined SHISHI
int kerberos = 0;

# ifdef ENCRYPTION
int encrypt_io = 0;
# endif	/* ENCRYPTION */
#endif /* KERBEROS */
int reverse_required = 0;
int debug_level = 0;

int numchildren;
int netf;
char line[1024];		/* FIXME */
int confirmed;
const char *path_login = PATH_LOGIN;
char *local_domain_name;
int local_dot_count;

struct winsize win = { 0, 0, 0, 0 };

#if defined __GLIBC__ && defined WITH_IRUSEROK
extern int iruserok (uint32_t raddr, int superuser,
                     const char *ruser, const char *luser);
#endif

void rlogin_daemon (int maxchildren, int port);
int rlogind_auth (int fd, struct auth_data *ap);
void setup_tty (int fd, struct auth_data *ap);
void exec_login (int authenticated, struct auth_data *ap);
int rlogind_mainloop (int infd, int outfd);
int do_rlogin (int infd, struct auth_data *ap);
int do_krb_login (int infd, struct auth_data *ap, const char **msg);
void getstr (int infd, char **ptr, const char *prefix);
void protocol (int f, int p, struct auth_data *ap);
int control (int pty, char *cp, size_t n);
void cleanup (int signo);
void fatal (int f, const char *msg, int syserr);
int in_local_domain (char *hostname);
char *topdomain (char *name, int max_dots);

void
rlogind_sigchld (int sig)
{
  pid_t pid;
  int status;

  while ((pid = waitpid (-1, &status, WNOHANG)) > 0)
    --numchildren;
  signal (sig, rlogind_sigchld);
}

#if defined KERBEROS && defined ENCRYPTION
# define ENCRYPT_IO encrypt_io
# define IF_ENCRYPT(stmt) if (encrypt_io) stmt
# define IF_NOT_ENCRYPT(stmt) if (!encrypt_io) stmt
# define ENC_READ(c, fd, buf, size, ap) \
 if (encrypt_io) \
     c = des_read(fd, buf, size); \
 else \
     c = read(fd, buf, size);
# define EN_WRITE(c, fd, buf, size, ap) \
 if (encrypt_io) \
     c = des_write(fd, buf, size); \
 else \
     c = write(fd, buf, size);
#elif defined(SHISHI) && defined (ENCRYPTION)
# define ENCRYPT_IO encrypt_io
# define IF_ENCRYPT(stmt) if (encrypt_io) stmt
# define IF_NOT_ENCRYPT(stmt) if (!encrypt_io) stmt
# define ENC_READ(c, fd, buf, size, ap) \
 if (encrypt_io) \
     readenc (ap->h, fd, buf, &c, &ap->iv1, ap->enckey, ap->protocol); \
 else \
     c = read(fd, buf, size);
# define ENC_WRITE(c, fd, buf, size, ap) \
 if (encrypt_io) \
     writeenc (ap->h, fd, buf, size, &c, &ap->iv2, ap->enckey, ap->protocol); \
 else \
     c = write(fd, buf, size);
#else
# define ENCRYPT_IO 0
# define IF_ENCRYPT(stmt)
# define IF_NOT_ENCRYPT(stmt) stmt
# define ENC_READ(c, fd, buf, size, ap) c = read (fd, buf, size)
# define ENC_WRITE(c, fd, buf, size, ap) c = write (fd, buf, size)
#endif


const char doc[] = "Remote login server";
const char *program_authors[] = {
  "Alain Magloire",
  "Sergey Poznyakoff",
  NULL
};

static struct argp_option options[] = {
  { "allow-root", 'o', NULL, 0,
    "allow uid == 0 to login, disabled by default" },
  { "verify-hostname", 'a', NULL, 0,
    "ask hostname for verification" },
  { "daemon", 'd', NULL, 0,
    "daemon mode" },
#ifdef HAVE___CHECK_RHOSTS_FILE
  { "no-rhosts", 'l', NULL, 0,
    "ignore .rhosts file" },
#endif
  { "no-keepalive", 'n', NULL, 0,
    "do not set SO_KEEPALIVE" },
  { "local-domain", 'L', "NAME", 0,
    "set local domain name" },
#if defined KERBEROS || defined SHISHI
  { "kerberos", 'k', NULL, 0,
    "use kerberos IV/V authentication" },
#endif
#if defined ENCRYPTION
  { "encrypt", 'x', NULL, 0,
    "use DES encryption" },
#endif
  { "debug", 'D', "LEVEL", OPTION_ARG_OPTIONAL,
    "set debug level" },
  { "port", 'p', "PORT", 0,
    "listen on given port (valid only in daemon mode)" },
  { "reverse-required", 'r', NULL, 0,
    "require reverse resolving of a remote host IP" },
  { NULL }
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'a':
      verify_hostname = 1;
      break;

    case 'D':
      if (arg)
	debug_level = strtoul (arg, NULL, 10);
      break;

    case 'd':
      mode = MODE_DAEMON;
      if (arg)
	maxchildren = strtoul (arg, NULL, 10);
      if (maxchildren == 0)
	maxchildren = DEFMAXCHILDREN;
      break;

#ifdef HAVE___CHECK_RHOSTS_FILE
    case 'l':
      __check_rhosts_file = 0;	/* FIXME: extern var? */
      break;
#endif

    case 'L':
      local_domain_name = arg;
      break;

    case 'n':
      keepalive = 0;
      break;

#if defined KERBEROS || defined SHISHI
    case 'k':
      if (arg)
	{
	  if (*arg == '4')
	    kerberos = AUTH_KERBEROS_4;
	  else if (*arg == '5')
	    kerberos = AUTH_KERBEROS_5;
	}
      else
	kerberos = AUTH_KERBEROS_DEFAULT;
      break;

# ifdef ENCRYPTION
    case 'x':
      encrypt_io = 1;
      break;
# endif	/* ENCRYPTION */
#endif /* KERBEROS */

    case 'o':
      allow_root = 1;
      break;

    case 'p':
      port = strtoul (arg, NULL, 10);
      break;

    case 'r':
      reverse_required = 1;
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}

static struct argp argp = { options, parse_opt, NULL, doc};



int
main (int argc, char *argv[])
{
  int index;

  set_program_name (argv[0]);

  /* Parse command line */
  iu_argp_init ("rlogind", program_authors);
  argp_parse (&argp, argc, argv, 0, &index, NULL);

  openlog ("rlogind", LOG_PID | LOG_CONS, LOG_AUTH);
  argc -= index;
  if (argc > 0)
    {
      syslog (LOG_ERR, "%d extra arguments", argc);
      exit (EXIT_FAILURE);
    }

  signal (SIGHUP, SIG_IGN);

  if (!local_domain_name)
    {
      char *p = localhost ();

      if (!p)
	{
	  syslog (LOG_ERR, "can't determine local hostname");
	  exit (EXIT_FAILURE);
	}
      local_dot_count = 2;
      local_domain_name = topdomain (p, local_dot_count);
    }
  else
    {
      char *p;

      local_dot_count = 0;
      for (p = local_domain_name; *p; p++)
	if (*p == '.')
	  local_dot_count++;
    }

  if (mode == MODE_DAEMON)
    rlogin_daemon (maxchildren, port);
  else
    exit (rlogind_mainloop (fileno (stdin), fileno (stdout)));

  return 0;
}


void
rlogin_daemon (int maxchildren, int port)
{
  pid_t pid;
  socklen_t size;
  struct sockaddr_in saddr;
  int listenfd, fd;

  if (port == 0)
    {
      struct servent *svp;

      svp = getservbyname ("login", "tcp");
      if (svp != NULL)
	port = ntohs (svp->s_port);
      else
	port = DEFPORT;
    }

  /* Become a daemon. Take care to close inherited fds and to hold
     first three one, lest master/slave ptys clash with standard
     in,out,err   */
  if (daemon (0, 0) < 0)
    {
      syslog (LOG_ERR, "failed to become a daemon %s", strerror (errno));
      fatal (fileno (stderr), "fork failed, exiting", 0);
    }

  signal (SIGCHLD, rlogind_sigchld);

  listenfd = socket (AF_INET, SOCK_STREAM, 0);

  if (listenfd == -1)
    {
      syslog (LOG_ERR, "socket: %s", strerror (errno));
      exit (EXIT_FAILURE);
    }

  {
    int on = 1;

    setsockopt (listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
  }

  size = sizeof saddr;
  memset (&saddr, 0, size);
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl (INADDR_ANY);
  saddr.sin_port = htons (port);

  size = sizeof saddr;
  if (bind (listenfd, (struct sockaddr *) &saddr, size) == -1)
    {
      syslog (LOG_ERR, "bind: %s", strerror (errno));
      exit (EXIT_FAILURE);
    }

  if (listen (listenfd, 128) == -1)
    {
      syslog (LOG_ERR, "listen: %s", strerror (errno));
      exit (EXIT_FAILURE);
    }

  while (1)
    {
      if (numchildren > maxchildren)
	{
	  syslog (LOG_ERR, "too many children (%d)", numchildren);
	  pause ();
	  continue;
	}

      size = sizeof saddr;
      fd = accept (listenfd, (struct sockaddr *) &saddr, &size);

      if (fd == -1)
	{
	  if (errno == EINTR)
	    continue;
	  syslog (LOG_ERR, "accept: %s", strerror (errno));
	  exit (EXIT_FAILURE);
	}

      pid = fork ();
      if (pid == -1)
	syslog (LOG_ERR, "fork: %s", strerror (errno));
      else if (pid == 0)	/* child */
	{
	  close (listenfd);
	  exit (rlogind_mainloop (fd, fd));
	}
      else
	numchildren++;
      close (fd);
    }
}

int
rlogind_auth (int fd, struct auth_data *ap)
{
  struct hostent *hp;
  char *hostname;
  int authenticated = 0;

#ifdef SHISHI
  int len, c;
#endif

  confirmed = 0;

  /* Check the remote host name */
  hp = gethostbyaddr ((char *) &ap->from.sin_addr, sizeof (struct in_addr),
		      ap->from.sin_family);
  if (hp)
    hostname = hp->h_name;
  else if (reverse_required)
    {
      syslog (LOG_CRIT, "can't resolve remote IP address");
      exit (EXIT_FAILURE);
    }
  else
    hostname = inet_ntoa (ap->from.sin_addr);

  ap->hostname = strdup (hostname);

  if (verify_hostname || in_local_domain (ap->hostname))
    {
      int match = 0;

      for (hp = gethostbyname (ap->hostname); hp && !match; hp->h_addr_list++)
	{
	  if (hp->h_addr_list[0] == NULL)
	    break;
	  match = memcmp (hp->h_addr_list[0], &ap->from.sin_addr,
			  sizeof (ap->from.sin_addr)) == 0;
	}
      if (!match)
	{
	  syslog (LOG_ERR | LOG_AUTH, "cannot find matching IP for %s (%s)",
		  ap->hostname, inet_ntoa (ap->from.sin_addr));
	  fatal (fd, "Permission denied", 0);
	}
    }

#if defined KERBEROS || defined SHISHI
  if (kerberos)
    {
      const char *err_msg;
      int c = 0;

      if (do_krb_login (fd, ap, &err_msg) == 0)
	authenticated++;
      else
	fatal (fd, err_msg, 0);
      write (fd, &c, 1);
      confirmed = 1;		/* we sent the null! */
    }
  else
#endif
    {
      int port = ntohs (ap->from.sin_port);

      if (ap->from.sin_family != AF_INET ||
	  port >= IPPORT_RESERVED || port < IPPORT_RESERVED / 2)
	{
	  syslog (LOG_NOTICE, "Connection from %s on illegal port %d",
		  inet_ntoa (ap->from.sin_addr), port);
	  fatal (fd, "Permission denied", 0);
	}
#ifdef IP_OPTIONS
      {
	unsigned char optbuf[BUFSIZ / 3], *cp;
	char lbuf[BUFSIZ], *lp;
	socklen_t optsize = sizeof (optbuf);
	int ipproto;
	struct protoent *ip;

	if ((ip = getprotobyname ("ip")) != NULL)
	  ipproto = ip->p_proto;
	else
	  ipproto = IPPROTO_IP;
	if (getsockopt (0, ipproto, IP_OPTIONS, (char *) optbuf,
			&optsize) == 0 && optsize != 0)
	  {
	    lp = lbuf;
	    for (cp = optbuf; optsize > 0; )
	      {
		sprintf (lp, " %2.2x", *cp);
		lp += 3;

		/* These two open an attack vector.  */
		if (*cp == IPOPT_SSRR || *cp == IPOPT_LSRR)
		  {
		    syslog (LOG_NOTICE,
			    "Discarding connection from %s with set source routing",
			    inet_ntoa (ap->from.sin_addr));
		    exit (EXIT_FAILURE);
		  }
		if (*cp == IPOPT_EOL)
		  break;
		if (*cp == IPOPT_NOP)
		  cp++, optsize--;
		else
		  {
		    /* Options using a length octet, see RFC 791.  */
		    int inc = cp[1];

		    optsize -= inc;
		    cp += inc;
		  }
	      }

	    syslog (LOG_NOTICE, "Ignoring IP options: %s", lbuf);
	    if (setsockopt (0, ipproto, IP_OPTIONS, (char *) NULL, optsize))
	      {
		syslog (LOG_ERR, "setsockopt IP_OPTIONS NULL: %m");
		exit (EXIT_FAILURE);
	      }
	  }
      }
#endif /* IP_OPTIONS */
      if (do_rlogin (fd, ap) == 0)
	authenticated++;
    }

  if (confirmed == 0)
    {
      write (fd, "", 1);
      confirmed = 1;		/* we sent the null! */
    }
#ifdef SHISHI
  len = sizeof (SECURE_MESSAGE) - 1;
  IF_ENCRYPT (writeenc
	      (ap->h, fd, SECURE_MESSAGE, len, &c, &ap->iv2, ap->enckey,
	       ap->protocol));
#else
  IF_ENCRYPT (des_write (fd, SECURE_MESSAGE, sizeof (SECURE_MESSAGE) - 1));
#endif
  return authenticated;
}

void
setup_tty (int fd, struct auth_data *ap)
{
  register char *cp = strchr (ap->term + ENVSIZE, '/');
  char *speed;
  struct termios tt;

  tcgetattr (fd, &tt);
  if (cp)
    {
      *cp++ = '\0';
      speed = cp;
      cp = strchr (speed, '/');
      if (cp)
	*cp++ = '\0';
#ifdef HAVE_CFSETSPEED
      cfsetspeed (&tt, strtoul (speed, NULL, 10));
#else
      cfsetispeed (&tt, strtoul (speed, NULL, 10));
      cfsetospeed (&tt, strtoul (speed, NULL, 10));
#endif
    }
  tt.c_iflag = TTYDEF_IFLAG;
  tt.c_oflag = TTYDEF_OFLAG;
  tt.c_lflag = TTYDEF_LFLAG;
  tcsetattr (fd, TCSAFLUSH, &tt);
  ap->env[0] = ap->term;
  ap->env[1] = 0;
}

#ifdef UTMPX
char *utmp_ptsid ();
 /*FIXME*/ void utmp_init ();

void
setup_utmp (char *line)
{
  char *ut_id = utmp_ptsid (line, "rl");

  utmp_init (line + sizeof ("/dev/") - 1, ".rlogin", ut_id);
}
#else
# define setup_utmp(line)
#endif

void
exec_login (int authenticated, struct auth_data *ap)
{
  if (authenticated)
    {
#ifdef SOLARIS
      execle (path_login, "login", "-p", "-s", "rlogin",
	      "-r", ap->hostname, "-U", ap->rusername,
	      ap->lusername, NULL, ap->env);
#else
      execle (path_login, "login", "-p",
	      "-h", ap->hostname, "-f", ap->lusername, NULL, ap->env);
#endif
    }
  else
    {
#ifdef SOLARIS
      execle (path_login, "login", "-p", "-s", "rlogin",
	      "-r", ap->hostname, "-U", ap->rusername,
	      ap->lusername, NULL, ap->env);
#else
      execle (path_login, "login", "-p",
	      "-h", ap->hostname, ap->lusername, NULL, ap->env);
#endif
    }
  syslog (LOG_ERR, "can't exec login: %m");
}

int
rlogind_mainloop (int infd, int outfd)
{
  socklen_t size;
  struct auth_data auth_data;
  int true;
  char c;
  int authenticated;
  pid_t pid;
  int master;

  memset (&auth_data, 0, sizeof auth_data);
  size = sizeof auth_data.from;
  if (getpeername (infd, (struct sockaddr *) &auth_data.from, &size) < 0)
    {
      syslog (LOG_ERR, "Can't get peer name of remote host: %m");
      fatal (outfd, "Can't get peer name of remote host", 1);
    }

  syslog (LOG_INFO, "Connect from %s:%d",
	  inet_ntoa (auth_data.from.sin_addr),
	  ntohs (auth_data.from.sin_port));

  true = 1;
  if (keepalive
      && setsockopt (infd, SOL_SOCKET, SO_KEEPALIVE, &true, sizeof true) < 0)
    syslog (LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");

#if defined IP_TOS && defined IPPROTO_IP && defined IPTOS_LOWDELAY
  true = IPTOS_LOWDELAY;
  if (setsockopt (infd, IPPROTO_IP, IP_TOS, (char *) &true, sizeof true) < 0)
    syslog (LOG_WARNING, "setsockopt (IP_TOS): %m");
#endif

  alarm (60);			/* Wait at most 60 seconds. FIXME: configurable? */

  /* Read the null byte */
  if (read (infd, &c, 1) != 1 || c != 0)
    {
      syslog (LOG_CRIT, "protocol error: expected 0 byte");
      exit (EXIT_FAILURE);
    }

  alarm (0);

  authenticated = rlogind_auth (infd, &auth_data);

  pid = forkpty (&master, line, NULL, &win);

  if (pid < 0)
    {
      if (errno == ENOENT)
	{
	  syslog (LOG_ERR, "Out of ptys");
	  fatal (infd, "Out of ptys", 0);
	}
      else
	{
	  syslog (LOG_ERR, "forkpty: %m");
	  fatal (infd, "Forkpty", 1);
	}
    }

  if (pid == 0)
    {
      /* Child */
      if (infd > 2)
	close (infd);

      setup_tty (0, &auth_data);
      setup_utmp (line);

      exec_login (authenticated, &auth_data);
      fatal (infd, "can't execute login", 1);
    }

  /* Parent */
  true = 1;
  IF_NOT_ENCRYPT (ioctl (infd, FIONBIO, &true));
  ioctl (master, FIONBIO, &true);
  ioctl (master, TIOCPKT, &true);
  netf = infd;			/* Needed for cleanup() */
  signal (SIGCHLD, cleanup);
  protocol (infd, master, &auth_data);
  signal (SIGCHLD, SIG_IGN);
  cleanup (0);

#ifdef SHISHI
  if (kerberos)
    {
      int i;

      shishi_done (auth_data.h);
# ifdef ENCRYPTION
      if (encrypt_io)
	{
	  shishi_key_done (auth_data.enckey);
	  for (i = 0; i < 2; i++)
	    {
	      shishi_crypto_close (auth_data.ivtab[i]->ctx);
	      free (auth_data.ivtab[i]->iv);
	    }
	}
# endif
    }
#endif

  return 0;
}


int
do_rlogin (int infd, struct auth_data *ap)
{
  struct passwd *pwd;
  int rc;

  getstr (infd, &ap->rusername, NULL);
  getstr (infd, &ap->lusername, NULL);
  getstr (infd, &ap->term, "TERM=");

  pwd = getpwnam (ap->lusername);
  if (pwd == NULL)
    {
      syslog (LOG_ERR, "no passwd entry for %s", ap->lusername);
      fatal (infd, "Permission denied", 0);
    }
  if (!allow_root && pwd->pw_uid == 0)
    {
      syslog (LOG_ERR, "root logins not permitted");
      fatal (infd, "Permission denied", 0);
    }

#ifdef WITH_IRUSEROK
  rc = iruserok (ap->from.sin_addr.s_addr, 0, ap->rusername, ap->lusername);
  if (rc)
    syslog (LOG_ERR, "iruserok failed: rusername=%s, lusername=%s",
	    ap->rusername, ap->lusername);
#elif defined WITH_RUSEROK
  rc = ruserok (inet_ntoa (ap->from.sin_addr), 0, ap->rusername, ap->lusername);
  if (rc)
    syslog (LOG_ERR, "ruserok failed: rusername=%s, lusername=%s",
	    ap->rusername, ap->lusername);
#else /* !WITH_IRUSEROK && !WITH_RUSEROK */
#error Unable to use mandatory iruserok/ruserok.  This should not happen.
#endif /* !WITH_IRUSEROK && !WITH_RUSEROK */

  return rc;
}

#if defined KERBEROS || defined SHISHI
int
do_krb_login (int infd, struct auth_data *ap, const char **err_msg)
{
  int rc;

  err_msg = NULL;
# if defined KRB5
  if (kerberos == AUTH_KERBEROS_5)
    rc = do_krb5_login (infd, ap, err_msg);
  else
# elif defined(SHISHI)
  if (kerberos == AUTH_KERBEROS_SHISHI)
    rc = do_shishi_login (infd, ap, err_msg);
  else
# else
  rc = do_krb4_login (infd, ap, err_msg);
# endif

  if (rc && !err_msg)
    *err_msg = kerberos_error_string (rc);

  return rc;
}

# ifdef KRB4
int
do_krb4_login (int infd, struct auth_data *ap, const char **err_msg)
{
  int rc;
  char instance[INST_SZ], version[VERSION_SZ];
  long authopts = 0L;		/* !mutual */
  struct sockaddr_in faddr;
  unsigned char auth_buf[sizeof (AUTH_DAT)];
  unsigned char tick_buf[sizeof (KTEXT_ST)];
  Key_schedule schedule;
  AUTH_DAT *kdata;
  KTEXT ticket;
  struct passwd *pwd;

  kdata = (AUTH_DAT *) auth_buf;
  ticket = (KTEXT) tick_buf;

  instance[0] = '*';
  instance[1] = '\0';

#  ifdef ENCRYPTION
  if (encrypt_io)
    {
      rc = sizeof faddr;
      if (getsockname (0, (struct sockaddr *) &faddr, &rc))
	{
	  *err_msg = "getsockname failed";
	  syslog (LOG_ERR, "getsockname failed: %m");
	  return 1;
	}
      authopts = KOPT_DO_MUTUAL;
      rc = krb_recvauth (authopts, 0,
			 ticket, "rcmd",
			 instance, &ap->from, &faddr,
			 kdata, "", schedule, version);
      des_set_key (kdata->session, schedule);

    }
  else
#  endif
    rc = krb_recvauth (authopts, 0,
		       ticket, "rcmd",
		       instance, &ap->from, NULL, kdata, "", NULL, version);

  if (rc != KSUCCESS)
    return 1;

  getstr (infd, &ap->lusername, NULL);
  /* get the "cmd" in the rcmd protocol */
  getstr (infd, &ap->term, "TERM=");

  pwd = getpwnam (ap->lusername);
  if (pwd == NULL)
    {
      *err_msg = "getpwnam failed";
      syslog (LOG_ERR, "getpwnam failed: %m");
      return 1;
    }
  /* returns nonzero for no access */
  if (kuserok (kdata, ap->lusername) != 0)
    return 1;

  if (pwd->pw_uid == 0)
    syslog (LOG_INFO | LOG_AUTH,
	    "ROOT Kerberos login from %s.%s@%s on %s\n",
	    kdata->pname, kdata->pinst, kdata->prealm, ap->hostname);
  else
    syslog (LOG_INFO | LOG_AUTH,
	    "%s Kerberos login from %s.%s@%s on %s\n",
	    pwd->pw_name,
	    kdata->pname, kdata->pinst, kdata->prealm, ap->hostname);

  return 0;
}
# endif

# ifdef KRB5
int
do_krb5_login (int infd, struct auth_data *ap, const char **err_msg)
{
  krb5_auth_context auth_ctx = NULL;
  krb5_error_code status;
  krb5_data inbuf;
  krb5_data version;
  krb5_authenticator *authenticator;
  krb5_rcache rcache;
  krb5_keyblock *key;
  krb5_ticket *ticket;
  struct sockaddr_in laddr;
  int len;
  struct passwd *pwd;
  char *name;

  if (status = krb5_init_context (&ap->context))
    {
      syslog (LOG_ERR, "Error initializing krb5: %s", error_message (status));
      return status;
    }

  if ((status = krb5_auth_con_init (ap->context, &auth_ctx))
      || (status = krb5_auth_con_genaddrs (ap->context, auth_ctx, infd,
					   KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR))
      || (status = krb5_auth_con_getrcache (ap->context, auth_ctx, &rcache)))
    return status;

  if (!rcache)
    {
      krb5_principal server;

      status = krb5_sname_to_principal (ap->context, 0, 0, KRB5_NT_SRV_HST,
					&server);
      if (status)
	return status;

      status = krb5_get_server_rcache (ap->context,
				       krb5_princ_component (ap->context,
							     server, 0),
				       &rcache);
      krb5_free_principal (ap->context, server);

      if (status)
	return status;

      status = krb5_auth_con_setrcache (ap->context, auth_ctx, rcache);
      if (status)
	return status;
    }

  len = sizeof (laddr);
  if (getsockname (infd, (struct sockaddr *) &laddr, &len))
    return errno;

  status = krb5_recvauth (ap->context, &auth_ctx, &infd, NULL, 0,
			  0, ap->keytab, &ticket);
  if (status)
    return status;

  if ((status = krb5_auth_con_getauthenticator (ap->context, auth_ctx,
						&authenticator)))
    return status;

  getstr (infd, &ap->lusername, NULL);
  getstr (infd, &ap->term, "TERM=");

  pwd = getpwnam (ap->lusername);
  if (pwd == NULL)
    {
      *err_msg = "getpwnam failed";
      syslog (LOG_ERR, "getpwnam failed: %m");
      return 1;
    }

  getstr (infd, &ap->rusername, NULL);

  if ((status = krb5_copy_principal (ap->context,
				     ticket->enc_part2->client, &ap->client)))
    return status;

  /*OK:: */
  if (ap->client && !krb5_kuserok (ap->context, ap->client, ap->lusername))
    return 1;

  krb5_unparse_name (ap->context, ap->client, &name);

  syslog (LOG_INFO | LOG_AUTH,
	  "%sKerberos V login from %s on %s\n",
	  (pwd->pw_uid == 0) ? "ROOT " : "", name, ap->hostname);
  free (name);

  return 0;
}

# endif

# ifdef SHISHI
int
do_shishi_login (int infd, struct auth_data *ad, const char **err_msg)
{
  int rc;
  int error = 0;
  int keylen, keytype;
  struct passwd *pwd = NULL;
  int cksumtype, cksumlen = 30;
  char *cksum;
  char *compcksum;
  size_t compcksumlen;
  char cksumdata[100];
  struct sockaddr_in sock;
  size_t socklen = sizeof (struct sockaddr_in);

#  ifdef ENCRYPTION
  rc = get_auth (infd, &ad->h, &ad->ap, &ad->enckey, err_msg, &ad->protocol,
		 &cksumtype, &cksum, &cksumlen);
#  else
  rc = get_auth (infd, &ad->h, &ad->ap, NULL, err_msg, &ad->protocol,
		 &cksumtype, &cksum, &cksumlen);
#  endif
  if (rc != SHISHI_OK)
    return rc;

#  ifdef ENCRYPTION
  /* init IV */
  if (encrypt_io)
    {
      int i;
      char *iv;

      ad->ivtab[0] = &ad->iv1;
      ad->ivtab[1] = &ad->iv2;

      keytype = shishi_key_type (ad->enckey);
      keylen = shishi_cipher_blocksize (keytype);

      for (i = 0; i < 2; i++)
	{
	  ad->ivtab[i]->ivlen = keylen;

	  switch (keytype)
	    {
	    case SHISHI_DES_CBC_CRC:
	    case SHISHI_DES_CBC_MD4:
	    case SHISHI_DES_CBC_MD5:
	    case SHISHI_DES_CBC_NONE:
	    case SHISHI_DES3_CBC_HMAC_SHA1_KD:
	      ad->ivtab[i]->keyusage = SHISHI_KEYUSAGE_KCMD_DES;
	      ad->ivtab[i]->iv = malloc (ad->ivtab[i]->ivlen);
	      memset (ad->ivtab[i]->iv, i, ad->ivtab[i]->ivlen);
	      ad->ivtab[i]->ctx =
		shishi_crypto (ad->h, ad->enckey, ad->ivtab[i]->keyusage,
			       shishi_key_type (ad->enckey), ad->ivtab[i]->iv,
			       ad->ivtab[i]->ivlen);
	      break;
	    case SHISHI_ARCFOUR_HMAC:
	    case SHISHI_ARCFOUR_HMAC_EXP:
	      ad->ivtab[i]->keyusage = SHISHI_KEYUSAGE_KCMD_DES + 6 - 4 * i;
	      ad->ivtab[i]->ctx =
		shishi_crypto (ad->h, ad->enckey, ad->ivtab[i]->keyusage,
			       shishi_key_type (ad->enckey), NULL, 0);
	      break;
	    default:
	      ad->ivtab[i]->keyusage = SHISHI_KEYUSAGE_KCMD_DES + 6 - 4 * i;
	      ad->ivtab[i]->iv = malloc (ad->ivtab[i]->ivlen);
	      memset (ad->ivtab[i]->iv, 0, ad->ivtab[i]->ivlen);
	      if (ad->protocol == 2)
		ad->ivtab[i]->ctx =
		  shishi_crypto (ad->h, ad->enckey, ad->ivtab[i]->keyusage,
				 shishi_key_type (ad->enckey),
				 ad->ivtab[i]->iv, ad->ivtab[i]->ivlen);
	    }
	}
    }
#  endif

  getstr (infd, &ad->lusername, NULL);
  getstr (infd, &ad->term, "TERM=");
  getstr (infd, &ad->rusername, NULL);

  rc = read (infd, &error, sizeof (int));
  if ((rc != sizeof (int)) && rc)
    {
      free (pwd);
      free (cksum);
      return 1;
    }

  /*
     getpwnam crash !!!!

     pwd = getpwnam (ad->lusername);
     if (pwd == NULL)
     {
     *err_msg = "getpwnam failed";
     syslog (LOG_ERR, "getpwnam failed: %m");
     return 1;
     }

     syslog (LOG_INFO | LOG_AUTH,
     "%sKerberos V login from %s on %s\n",
     (pwd->pw_uid == 0) ? "ROOT " : "",
     ad->lusername, ad->hostname);

   */

  free (pwd);

  syslog (LOG_INFO | LOG_AUTH,
	  "Kerberos V login from %s on %s\n", ad->lusername, ad->hostname);

  /* verify checksum */

  if (getsockname (infd, (struct sockaddr *) &sock, &socklen) < 0)
    {
      syslog (LOG_ERR, "Can't get sock name");
      fatal (infd, "Can't get sockname", 1);
    }

  snprintf (cksumdata, 100, "%u:%s%s", ntohs (sock.sin_port), ad->term + 5,
	    ad->lusername);
  rc = shishi_checksum (ad->h, ad->enckey, 0, cksumtype, cksumdata,
			strlen (cksumdata), &compcksum, &compcksumlen);
  free (cksum);
  if (rc != SHISHI_OK
      || compcksumlen != cksumlen || memcmp (compcksum, cksum, cksumlen) != 0)
    {
      /* err_msg crash ? */
      /* *err_msg = "checksum verify failed"; */
      syslog (LOG_ERR, "checksum verify failed: %s", shishi_error (ad->h));
      free (compcksum);
      return 1;
    }

  free (compcksum);

  rc = shishi_authorized_p (ad->h, shishi_ap_tkt (ad->ap), ad->lusername);
  if (!rc)
    {
      syslog (LOG_ERR, "User is not authorized to log in as: %s",
	      ad->lusername);
      shishi_ap_done (ad->ap);
      return 1;
    }

  shishi_ap_done (ad->ap);

  return SHISHI_OK;
}
# endif
#endif

#define BUFFER_SIZE 128

void
getstr (int infd, char **ptr, const char *prefix)
{
  char c;
  char *buf;
  int pos;
  int size = BUFFER_SIZE;

  if (prefix)
    {
      int len = strlen (prefix);

      if (size < len + 1)
	size = len + 1;
    }

  buf = malloc (size);
  if (!buf)
    {
      syslog (LOG_ERR, "not enough memory");
      exit (EXIT_FAILURE);
    }

  pos = 0;
  if (prefix)
    {
      strcpy (buf, prefix);
      pos += strlen (buf);
    }

  do
    {
      if (read (infd, &c, 1) != 1)
	{
	  syslog (LOG_ERR, "read error: %m");
	  exit (EXIT_FAILURE);
	}
      if (pos == size)
	{
	  size += BUFFER_SIZE;
	  buf = realloc (buf, size);
	  if (!buf)
	    {
	      syslog (LOG_ERR, "not enough memory");
	      exit (EXIT_FAILURE);
	    }
	}
      buf[pos++] = c;
    }
  while (c != 0);
  *ptr = buf;
}

#define pkcontrol(c) ((c)&(TIOCPKT_FLUSHWRITE|TIOCPKT_NOSTOP|TIOCPKT_DOSTOP))

char magic[2] = { 0377, 0377 };
char oobdata[] = { TIOCPKT_WINDOW };	/* May be modified by protocol/control */

#ifdef SHISHI
char oobdata_new[] = { 0377, 0377, 'o', 'o', TIOCPKT_WINDOW };
#endif

void
protocol (int f, int p, struct auth_data *ap)
{
  char fibuf[1024], *pbp = NULL, *fbp = NULL;
  int pcc = 0, fcc = 0;
  int cc, nfd, n;
  char cntl;

  /*
   * Must ignore SIGTTOU, otherwise we'll stop
   * when we try and set slave pty's window shape
   * (our controlling tty is the master pty).
   */
  signal (SIGTTOU, SIG_IGN);
#ifdef SHISHI
  if (kerberos && (ap->protocol == 2))
    {
      ENC_WRITE (n, f, oobdata_new, 5, ap);
    }
  else
#endif
    send (f, oobdata, 1, MSG_OOB);	/* indicate new rlogin */
  if (f > p)
    nfd = f + 1;
  else
    nfd = p + 1;
  if (nfd > FD_SETSIZE)
    {
      syslog (LOG_ERR, "select mask too small, increase FD_SETSIZE");
      fatal (f, "internal error (select mask too small)", 0);
    }

  while (1)
    {
      fd_set ibits, obits, ebits, *omask;

      FD_ZERO (&ebits);
      FD_ZERO (&ibits);
      FD_ZERO (&obits);
      omask = (fd_set *) NULL;

      if (fcc)
	{
	  FD_SET (p, &obits);
	  omask = &obits;
	}
      else
	FD_SET (f, &ibits);

      if (pcc >= 0)
	{
	  if (pcc)
	    {
	      FD_SET (f, &obits);
	      omask = &obits;
	    }
	  else
	    FD_SET (p, &ibits);
	}

      FD_SET (p, &ebits);

      if ((n = select (nfd, &ibits, omask, &ebits, 0)) < 0)
	{
	  if (errno == EINTR)
	    continue;
	  fatal (f, "select", 1);
	}
      if (n == 0)
	{
	  /* shouldn't happen... */
	  sleep (5);
	  continue;
	}

      if (FD_ISSET (p, &ebits))
	{
	  cc = read (p, &cntl, 1);
	  if (cc == 1 && pkcontrol (cntl))
	    {
	      cntl |= oobdata[0];
	      send (f, &cntl, 1, MSG_OOB);
	      if (cntl & TIOCPKT_FLUSHWRITE)
		{
		  pcc = 0;
		  FD_CLR (p, &ibits);
		}
	    }
	}

      if (FD_ISSET (f, &ibits))
	{
	  ENC_READ (fcc, f, fibuf, sizeof (fibuf), ap);

	  if (fcc < 0 && errno == EWOULDBLOCK)
	    fcc = 0;
	  else
	    {
	      register char *cp;
	      int left;

	      if (fcc <= 0)
		break;
	      fbp = fibuf;

	      for (cp = fibuf; cp < fibuf + fcc - 1; cp++)
		if (cp[0] == magic[0] && cp[1] == magic[1])
		  {
		    int len;

		    left = fcc - (cp - fibuf);
		    len = control (p, cp, left);
		    if (len)
		      {
			left -= len;
			if (left > 0)
			  memmove (cp, cp + len, left);
			fcc -= len;
			cp--;
		      }
		  }
	      FD_SET (p, &obits);	/* try write */
	    }
	}

      if (FD_ISSET (p, &obits) && fcc > 0)
	{
	  cc = write (p, fbp, fcc);
	  if (cc > 0)
	    {
	      fcc -= cc;
	      fbp += cc;
	    }
	}

      if (FD_ISSET (p, &ibits))
	{
	  char dbuf[1024 + 1];

	  pcc = read (p, dbuf, sizeof dbuf);

	  pbp = dbuf;
	  if (pcc < 0)
	    {
	      if (errno == EWOULDBLOCK)
		pcc = 0;
	      else
		break;
	    }
	  else if (pcc == 0)
	    {
	      break;
	    }
	  else if (dbuf[0] == 0)
	    {
	      pbp++;
	      pcc--;
	      IF_NOT_ENCRYPT (FD_SET (f, &obits));	/* try write */
	    }
	  else
	    {
	      if (pkcontrol (dbuf[0]))
		{
		  dbuf[0] |= oobdata[0];
		  send (f, &dbuf[0], 1, MSG_OOB);
		}
	      pcc = 0;
	    }
	}

      if ((FD_ISSET (f, &obits)) && pcc > 0)
	{
	  ENC_WRITE (cc, f, pbp, pcc, ap);

	  if (cc < 0 && errno == EWOULDBLOCK)
	    {
	      /*
	       * This happens when we try write after read
	       * from p, but some old kernels balk at large
	       * writes even when select returns true.
	       */
	      if (!FD_ISSET (p, &ibits))
		sleep (5);
	      continue;
	    }
	  if (cc > 0)
	    {
	      pcc -= cc;
	      pbp += cc;
	    }
	}
    }
}

/* Handle a "control" request (signaled by magic being present)
   in the data stream.  For now, we are only willing to handle
   window size changes. */
int
control (int pty, char *cp, size_t n)
{
  struct winsize w;

  if (n < 4 + sizeof (w) || cp[2] != 's' || cp[3] != 's')
    return (0);
  oobdata[0] &= ~TIOCPKT_WINDOW;	/* we know he heard */
  memmove (&w, cp + 4, sizeof w);
  w.ws_row = ntohs (w.ws_row);
  w.ws_col = ntohs (w.ws_col);
  w.ws_xpixel = ntohs (w.ws_xpixel);
  w.ws_ypixel = ntohs (w.ws_ypixel);
  ioctl (pty, TIOCSWINSZ, &w);
  return (4 + sizeof w);
}

void
cleanup (int signo _GL_UNUSED_PARAMETER)
{
  char *p;

  p = line + sizeof (PATH_DEV) - 1;
#ifdef UTMPX
  utmp_logout (p);
  chmod (line, 0644);
  chown (line, 0, 0);
#else
  if (logout (p))
    logwtmp (p, "", "");
  chmod (line, 0666);
  chown (line, 0, 0);
  *p = 'p';
  chmod (line, 0666);
  chown (line, 0, 0);
#endif
  shutdown (netf, 2);
  exit (EXIT_FAILURE);
}

int
in_local_domain (char *hostname)
{
  char *p = topdomain (hostname, local_dot_count);

  return p && strcasecmp (p, local_domain_name) == 0;
}

char *
topdomain (char *name, int max_dots)
{
  char *p;
  int dot_count = 0;

  for (p = name + strlen (name) - 1; p >= name; p--)
    {
      if (*p == '.' && ++dot_count == max_dots)
	return p + 1;
    }
  return name;
}

void
fatal (int f, const char *msg, int syserr)
{
  int len;
  char buf[BUFSIZ], *bp = buf;

  /*
   * Prepend binary one to message if we haven't sent
   * the magic null as confirmation.
   */
  if (!confirmed)
    *bp++ = '\01';		/* error indicator */
  if (syserr)
    snprintf (bp, sizeof buf - (bp - buf),
	      "rlogind: %s: %s.\r\n", msg, strerror (errno));
  else
    snprintf (bp, sizeof buf - (bp - buf), "rlogind: %s.\r\n", msg);
  len = strlen (bp);
  write (f, buf, bp + len - buf);
  exit (EXIT_FAILURE);
}
