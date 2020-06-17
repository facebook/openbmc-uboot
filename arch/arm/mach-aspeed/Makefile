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

obj-y += timer.o reset.o cpuinfo.o ast-scu.o ast-ahbc.o ast-sdmc.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_AST_SPI_NOR) += flash.o
endif
ifdef CONFIG_ARCH_AST2500
obj-y += platform_g5.o
ifdef CONFIG_ASPEED_SPI_DMA
obj-y += utils.o
endif
endif
obj-$(CONFIG_ARCH_AST2400) += platform_g4.o
