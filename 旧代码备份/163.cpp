
/////////////////////////////////////////////////////////////////////
// Mapper 163

void NES_mapper163_Init()
{
	g_NESmapper.Reset = NES_mapper163_Reset;
	g_NESmapper.MemoryWriteLow = NES_mapper163_MemoryWriteLow;
	g_NESmapper.MemoryReadLow = NES_mapper163_MemoryReadLow;
	g_NESmapper.HSync = NES_mapper163_HSync;
}


void NES_mapper163_Reset()
{
	g_NESmapper.Mapper163.regs[1] = 0xFF;
	g_NESmapper.Mapper163.strobe = 1;
	g_NESmapper.Mapper163.security = g_NESmapper.Mapper163.trigger = g_NESmapper.Mapper163.regs[0] = 0x00;
	g_NESmapper.Mapper163.rom_type = 0;
	g_NESmapper.SetPROM_32K_Bank(15);

	if( (NES_crc32() == 0xb6a10d5d)||
		(NES_crc32() == 0x09C7AED3) ){	// Hu Lu Jin Gang (NJ039) (Ch) [dump]
		g_NESmapper.SetPROM_32K_Bank(0);
	}
	
	if( NES_crc32() == 0xf52468e7 ) {	// San Guo Wu Shuang - Meng Jiang Zhuan (NJ047) (Ch) [dump]
		g_NESmapper.Mapper163.rom_type = 1;
	}
	
	g_PPU.vram_write_protect = 0;
}

uint8 NES_mapper163_MemoryReadLow(uint32 addr)
{
	if((addr>=0x5000 && addr<0x6000))
	{
		switch (addr & 0x7700)
		{
			case 0x5100:
					return g_NESmapper.Mapper163.security;
					break;
			case 0x5500:
					if(g_NESmapper.Mapper163.trigger)
						return g_NESmapper.Mapper163.security;
					else
						return 0;
					break;
		}
		return 4;
	}
	else if( addr>=0x6000 ) {
		//return	CPU_MEM_BANK[addr>>13][addr&0x1FFF];
		nes6502_context context;
		NES6502_GetContext(&context);
		return context.mem_page[addr>>13][addr&0x1FFF];
	}
	//return NES_mapper163_MemoryReadLow( addr );
	return  (uint8)(addr >> 8);
}

void NES_mapper163_MemoryWriteLow(uint32 addr, uint8 data)
{
	if((addr>=0x4020 && addr<0x6000))
	{
		if(addr==0x5101){
			if(g_NESmapper.Mapper163.strobe && !data){
				g_NESmapper.Mapper163.trigger ^= 1;
			}
			g_NESmapper.Mapper163.strobe = data;
		}else if(addr==0x5100 && data==6){
			g_NESmapper.SetPROM_32K_Bank(3);
		}
		else{
			switch (addr & 0x7300)
			{
				case 0x5000:
						g_NESmapper.Mapper163.regs[1]=data;
						g_NESmapper.SetPROM_32K_Bank( (g_NESmapper.Mapper163.regs[1] & 0xF) | (g_NESmapper.Mapper163.regs[0] << 4) );

						if(!(g_NESmapper.Mapper163.regs[1]&0x80)&&(NES_PPU_GetScanlineNo()<128))
						{
							g_NESmapper.SetCRAM_8K_Bank(0);
						}
						if(g_NESmapper.Mapper163.rom_type==1)
						{
							g_NESmapper.SetCRAM_8K_Bank(0);
						}
						break;
				case 0x5200:
						g_NESmapper.Mapper163.regs[0]=data;
						g_NESmapper.SetPROM_32K_Bank( (g_NESmapper.Mapper163.regs[1] & 0xF) | (g_NESmapper.Mapper163.regs[0] << 4) );
						break;
				case 0x5300:
						g_NESmapper.Mapper163.security=data;
						break;
			}
		}
	}
	else if( addr>=0x6000 ) {
		//CPU_MEM_BANK[addr>>13][addr&0x1FFF] = data;	
		nes6502_context context;
		NES6502_GetContext(&context);
		context.mem_page[addr>>13][addr&0x1FFF] = data;//g_NESmapper.Mapper163.Saveram;
	}
}


void NES_mapper163_HSync(uint32 scanline)
{
	if( g_NESmapper.Mapper163.regs[1]&0x80 )
	if( NES_PPU_spr_enabled() || NES_PPU_bg_enabled() )
	{
		if(scanline==128)
		{
			g_NESmapper.SetCRAM_4K_Bank(0, 1);
			g_NESmapper.SetCRAM_4K_Bank(4, 1);	
		}
		if (g_NESmapper.Mapper163.rom_type==1)
		{
			if(scanline<128){
				g_NESmapper.SetCRAM_4K_Bank(0, 0);
				g_NESmapper.SetCRAM_4K_Bank(4, 0);
			}
		}else{
			if(scanline==239){
				g_NESmapper.SetCRAM_4K_Bank(0, 0);
				g_NESmapper.SetCRAM_4K_Bank(4, 0);
			}
		}	
	}
}
