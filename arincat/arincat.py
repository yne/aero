import time
from ctypes import *

class RESET_CMD(Structure):_fields_ = [("res_ctl",c_uint8),("glob_speed_mod",c_uint8),("low_speed_sel",c_uint8),("tx_clock_sel",c_uint8),("sm",c_uint32),("lcen",c_uint32),("mbuf_size",c_uint32),("chn_cfg",c_uint32),]
class RESET_ACK(Structure):_fields_ = [("chn_cnt",c_uint16),("padding1",c_uint16),("pchc",c_uint32),("txrx_mem_start",c_uint32),("txrx_mem_end",c_uint32),("res1",c_uint32),("res2",c_uint32),("rm_mem_start",c_uint32*2*16),("rm_mem_size",c_uint32*2*16),]
class RCV_ENTRY(Structure):_fields_ = [("label",c_uint32,8),("data",c_uint32,21),("SSM",c_uint32,2),("parity",c_uint32,1),("tm_tag",c_uint32),("brw",c_uint32),]
class RCV_TRG_MODE(Structure):_fields_ = [("tmod",c_uint8),("exti",c_uint8),("imod",c_uint8),("ext",c_uint8),("exto",c_uint8),("exta",c_uint8),("extad",c_uint8),("reserved",c_uint8),]
class RCV_CAP_MODE(Structure):_fields_ = [("mode",c_uint8),("padding1",c_uint8),("padding2",c_uint16),("tat",c_uint16),("reserved",c_uint16),("rec_filesize",c_uint32)]

class ARINC(object):
	def __init__(self, card_number=0):
		self.dll = WinDLL("api_civ")#and not CDLL since it use the windows EABI
		#assert self.dll.Api429Init()>0, "No card found"
		self.dll.Api429Init()
		self.fd = c_uint8()
		assert not self.dll.Api429Open(c_uint8(card_number), "local", byref(self.fd))
		assert not self.dll.Api429CmdIni(self.fd,byref(create_string_buffer(2*1+5*2+(13+16)*4)))
		#todo debug this one
		self.dll.Api429CmdReset(self.fd, byref(RESET_CMD(lcen=0xffff,mbuf_size=0x100,res_ctl=0)), byref(RESET_ACK()))
	def start(self,channel,labels=range(255)):
		assert not self.dll.Api429CmdRmIni(self.fd, channel, 0)
		assert not self.dll.Api429CmdRmTrgDef(self.fd, channel, byref(RCV_TRG_MODE(tmod=3)))
		assert not self.dll.Api429CmdRmCapDef(self.fd, channel, byref(RCV_CAP_MODE()))
		assert not self.dll.Api429CmdDefChnSpeed(self.fd, channel,0)#0:Low 1:High
		assert not self.dll.Api429CmdRxStart(self.fd, channel)
		for i in labels:
			assert not self.dll.Api429CmdRmLabCon(self.fd, channel, i, 0, 1)
		try:
			pool_size = 128#should be enough
			pool = (RCV_ENTRY*pool_size)()
			got = c_uint16()
			while True:
				assert not self.dll.Api429CmdRmDataRead(self.fd, channel, c_uint16(pool_size), byref(got), byref(pool))
				for i in range(got.value):
					print "%03o %06X"%(pool[i].label,pool[i].data)
				time.sleep(.01);
		except KeyboardInterrupt:
			self.stop(channel)
	def stop(self,channel):
		assert not self.dll.Api429CmdRxHalt(self.fd, channel)
