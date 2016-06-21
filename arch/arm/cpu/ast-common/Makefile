#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libcpuast-common.o

COBJS-y	+= timer.o reset.o cpuinfo.o ast-scu.o ast-ahbc.o ast-sdmc.o watchdog.o
COBJS-$(CONFIG_ARCH_AST1070)	+= ast1070-scu.o
COBJS-$(CONFIG_PCI)		+= pcie.o ast-p2x.o
COBJS-$(CONFIG_AST_SPI_NOR)	+= flash.o
COBJS-$(CONFIG_CMD_NETTEST)	+= MACTEST.o DRAM_SPI.o IO.o LIB.o MAC.o NCSI.o PCI_SPI.o PHY.o SPIM.o LAN9303.o STDUBOOT.o
COBJS-$(CONFIG_CMD_SLT)		+= STRESS.o

SRCS	:= $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
