#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <pci.h>
#include <linux/sockios.h>

/* hack, so we may include kernel's ethtool.h */
typedef unsigned long long __u64;
typedef __uint32_t __u32;         /* ditto */
typedef __uint16_t __u16;         /* ditto */
typedef __uint8_t __u8;           /* ditto */

#include <linux/ethtool.h>

static char* fwnet_ifbusid(const char *iface)
{
	FILE	*fp = NULL;
	char	path[PATH_MAX] = "";
	char	*ret = NULL;

	snprintf(path, PATH_MAX-1, "/sys/class/net/%s/uevent", iface);
	fp = fopen(path, "r");
	if (fp != NULL)
	{
		char line[PATH_MAX] = "";
		while (fgets(line,PATH_MAX-1,fp))
		{
			char *tok = NULL;
			tok = strtok(line, "=");
			if (!strcmp(tok,"PHYSDEVPATH"))
			{
				char *tmp = NULL;
				tmp = strtok(NULL, "=");
				if (tmp != NULL)
				{
					ret = strdup((char*)basename(tmp));
				}		
			}
		}
		fclose(fp);
		/* strip the trailing newline */
		ret[strlen(ret)-1] = 0;
	}

	return ret;
}

int fwnet_ifdesc(const char *iface, char *desc, int size)
{
	struct ifreq ifr;
	int fd, err, len, device, vendor;
	struct ethtool_drvinfo drvinfo;
	char buf[512], path[PATH_MAX];
	struct pci_access *pacc;

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, iface);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		perror("Cannot get control socket");
		return 1;
	}
	drvinfo.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t)&drvinfo;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	close(fd);
	if (err < 0)
	{
		/* try the alternate method of fetching bus info */
		char *tmp = fwnet_ifbusid (iface);
		if (tmp != NULL)
		{
			strncpy(drvinfo.bus_info, tmp, 32);
			free(tmp);
		}
		else
		{
			perror("Cannot get driver information");
			printf("%d\n", errno);
			return 2;
		}
	}
	snprintf(path, PATH_MAX-1, "/sys/bus/pci/devices/%s/vendor", drvinfo.bus_info);
	fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		perror("Cannot open the vendor file");
		return 3;
	}
	len = read(fd, buf, sizeof(buf));
	buf[len-1] = '\0';
	close(fd);
	sscanf(buf,"%X", &vendor);
	snprintf(path, PATH_MAX, "/sys/bus/pci/devices/%s/device", drvinfo.bus_info);
	fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		perror("Cannot open the device file");
		return 3;
	}
	len = read(fd, buf, sizeof(buf));
	buf[len-1] = '\0';
	close(fd);
	sscanf(buf,"%X", &device);
	pacc = pci_alloc();
	pci_init(pacc);
	pci_lookup_name(pacc, desc, size,
			PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
			vendor, device);
	pci_cleanup(pacc);
	return(0);
}
