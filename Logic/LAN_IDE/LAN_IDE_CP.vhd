----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    22:08:29 12/13/2016 
-- Design Name: 
-- Module Name:    LAN_IDE_CP - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
-- directory cleanup hint:
--  svn status|grep ^M|sed "s/^M [\t ]*//"|xargs rm
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity LAN_IDE_CP is
    Port ( A : inout  STD_LOGIC_VECTOR (23 downto 1);
           D : inout  STD_LOGIC_VECTOR (15 downto 0);
           DQ : inout  STD_LOGIC_VECTOR (15 downto 0);
           A_LAN : out  STD_LOGIC_VECTOR (13 downto 0);
           OWN : out  STD_LOGIC;
           SLAVE : out  STD_LOGIC;
           CFOUT : out  STD_LOGIC;
           CFIN : in  STD_LOGIC;
           C1 : in  STD_LOGIC;
           C3 : in  STD_LOGIC;
           MTCR : in  STD_LOGIC;
           OVR : out  STD_LOGIC;
           BERR : in  STD_LOGIC;
           MTACK : out  STD_LOGIC;
           DS0 : in  STD_LOGIC;
           DTACK : out  STD_LOGIC;
           UDS : in  STD_LOGIC;
           LDS : in  STD_LOGIC;
           AS : in  STD_LOGIC;
           RW : in  STD_LOGIC;
           Z3 : in  STD_LOGIC;
           DS1 : in  STD_LOGIC;
           FCS : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           INT2_OUT : out  STD_LOGIC;
			  INT6_OUT : out  STD_LOGIC;
           AUTOBOOT_OFF : in  STD_LOGIC;
           ROM_B : out  STD_LOGIC_VECTOR (1 downto 0);
           ROM_OE : out  STD_LOGIC;
           IDE_WAIT : in  STD_LOGIC;
           CLK_EXT : in  STD_LOGIC;
           IDE_W : out  STD_LOGIC;
           IDE_R : out  STD_LOGIC;
           IDE_A : out  STD_LOGIC_VECTOR (2 downto 0);
           IDE_CS : out  STD_LOGIC_VECTOR (1 downto 0);
           LAN_CFG : out  STD_LOGIC_VECTOR (4 downto 1);
           LAN_RD : out  STD_LOGIC;
           LAN_CS : out  STD_LOGIC;
           LAN_WRH : out  STD_LOGIC;
           LAN_WRL : out  STD_LOGIC;
           LAN_INT : in  STD_LOGIC;
           CP_RD : out  STD_LOGIC;
           CP_WE : out  STD_LOGIC;
           CP_CS : out  STD_LOGIC;
           CP_IRQ : in  STD_LOGIC);
end LAN_IDE_CP;

architecture Behavioral of LAN_IDE_CP is

	TYPE lan_reset IS (
				nop,
				wait0,
				clr,
				clr_commit,
				wait1,
				set,
				set_commit,
				done
	);

	TYPE lan_bus_sm IS (
				nop,
				start_read_upper,
				wait_read_upper,
				end_read_upper,
				start_read_lower,
				wait_read_lower,
				end_read_lower,
				start_write_upper,
				wait_write_upper,
				end_write_upper,
				start_write_lower,
				wait_write_lower,
				end_write_lower,
				config_rw
	);

	signal LAN_RST_SM: lan_reset :=nop;
	signal LAN_SM: lan_bus_sm :=nop;
	signal AUTOCONFIG_Z2_ACCESS: STD_LOGIC;
	signal IDE_ACCESS: STD_LOGIC;
	signal IDE_ENABLE: STD_LOGIC;
	signal IDE_BASEADR:STD_LOGIC_VECTOR(7 downto 0);
	signal IDE_R_S: STD_LOGIC;
	signal IDE_W_S: STD_LOGIC;
	signal ROM_OE_S: STD_LOGIC;
	
	signal CP_ACCESS: STD_LOGIC;
	signal CP_BASEADR:STD_LOGIC_VECTOR(7 downto 0);
	signal LAN_ACCESS: STD_LOGIC;
	signal DQ_SWAP: STD_LOGIC;
	signal D_Z2_OUT:STD_LOGIC_VECTOR(3 downto 0):=x"F";
	signal AUTO_CONFIG_Z2_DONE:STD_LOGIC_VECTOR(2 downto 0):="000";
	signal AUTO_CONFIG_Z2_DONE_CYCLE:STD_LOGIC_VECTOR(2 downto 0):="000";
	signal SHUT_UP_Z2:STD_LOGIC_VECTOR(2 downto 0):="111";
	signal LAN_BASEADR:STD_LOGIC_VECTOR(15 downto 0);
	signal LAN_INT_ENABLE: std_logic :='0';
	signal LAN_RD_S: std_logic;
	signal LAN_WRH_S: std_logic;
	signal LAN_WRL_S: std_logic;
	signal LAN_READY: std_logic;
	signal CONFIG_READY: std_logic;
	signal LAN_IRQ_D0: std_logic;
	signal LAN_IRQ_OUT: std_logic;
	signal Z3_ADR:STD_LOGIC_VECTOR(15 downto 2);
	signal Z3_DATA_IN:STD_LOGIC_VECTOR(31 downto 0);
	signal Z3_DATA:STD_LOGIC_VECTOR(31 downto 0);
	signal DQ_DATA:STD_LOGIC_VECTOR(15 downto 0);
	signal Z3_DS:STD_LOGIC_VECTOR(3 downto 0);
	signal Z3_A_LOW:STD_LOGIC;
	signal LAN_SM_RST:STD_LOGIC;

	signal CP_RD_S: std_logic;
	signal CP_WE_S: std_logic;
	signal CP_WE_QUIRK: std_logic;

	signal AMIGA_CLK:STD_LOGIC;
	signal DS:STD_LOGIC;
	
	signal LAN_CS_RST: std_logic;
	signal LAN_WR_RST: std_logic;
	signal INT_VECTOR_CYCLE: std_logic;
	
	signal LAN_A_INIT:STD_LOGIC_VECTOR(13 downto 0) :="11"&x"FFF";
	signal LAN_D_INIT:STD_LOGIC_VECTOR(15 downto 0) := x"0000";
	constant LAN_A_CLRREG:STD_LOGIC_VECTOR(13 downto 0) :="11"&x"FF7";
	constant LAN_A_SETREG:STD_LOGIC_VECTOR(13 downto 0) :="11"&x"FB7";
	constant LAN_D_SET:STD_LOGIC_VECTOR(15 downto 0) :=   "0000001000010010"; --25MHz SET-MASK	
	constant LAN_D_CLR:STD_LOGIC_VECTOR(15 downto 0) :=   "0000110100000000"; --25MHz CLEAR-MASK
	
   Function to_std_logic(X: in Boolean) return Std_Logic is
   variable ret : std_logic;
   begin
   if x then ret := '1';  else ret := '0'; end if;
   return ret;
   end to_std_logic;
	
begin

	Z3_DATA_IN <= D(15 downto 0) & A(23 downto 8);
	
	Z3_DS <= UDS & LDS & DS1 & DS0;
	AMIGA_CLK <= not (C1 xor C3);
	DS <= UDS and LDS;
	
	
	clock_init: process(reset,CLK_EXT)
	begin
		if(reset='1')then
			--default values
			LAN_CS_RST<='0';
			LAN_WR_RST<='0';						
			LAN_RST_SM<=nop;
			LAN_A_INIT<="11"&x"FFF";
			LAN_D_INIT<= x"0000";
		elsif(rising_edge(CLK_EXT))then --reset is low!			
			case LAN_RST_SM is
				when nop=>
					LAN_CS_RST<='0';
					LAN_WR_RST<='0';						
					LAN_RST_SM<=wait0;
					LAN_A_INIT <= LAN_A_CLRREG; 
					LAN_D_INIT <= LAN_D_CLR;
				when wait0=>
					LAN_CS_RST<='1';
					LAN_WR_RST<='0';						
					LAN_RST_SM<=clr;
					LAN_A_INIT <= LAN_A_CLRREG;
					LAN_D_INIT <= LAN_D_CLR;
				when clr=>
					LAN_CS_RST<='1';
					LAN_WR_RST<='1';						
					LAN_RST_SM<=clr_commit;
					LAN_A_INIT <= LAN_A_CLRREG;
					LAN_D_INIT <= LAN_D_CLR;
				when clr_commit=>
					LAN_CS_RST<='1';
					LAN_WR_RST<='0';						
					LAN_RST_SM<=wait1;
					LAN_A_INIT <= LAN_A_CLRREG;
					LAN_D_INIT <= LAN_D_CLR;
				when wait1=>
					LAN_CS_RST<='1';
					LAN_WR_RST<='0';						
					LAN_RST_SM<=set;
					LAN_A_INIT <= LAN_A_SETREG;
					LAN_D_INIT <= LAN_D_SET;
				when 	set=>
					LAN_CS_RST<='1';
					LAN_WR_RST<='1';						
					LAN_RST_SM<=set_commit;
					LAN_A_INIT <= LAN_A_SETREG;
					LAN_D_INIT <= LAN_D_SET;
				when 	set_commit=>
					LAN_CS_RST<='1';
					LAN_WR_RST<='0';						
					LAN_RST_SM<=done;
					LAN_A_INIT <= LAN_A_SETREG;
					LAN_D_INIT <= LAN_D_SET;
				when 	done=>
					LAN_CS_RST<='0';
					LAN_WR_RST<='0';						
					LAN_RST_SM<=done;
					LAN_A_INIT <= LAN_A_SETREG;
					LAN_D_INIT <= LAN_D_SET;
			end case;
		end if;
	end process clock_init;
	
	ADDRESS_Z3_DECODE: process(reset,FCS)
	begin
		if(reset='0')then
			LAN_ACCESS 		<= '0';
			DQ_SWAP  <= '1';
			Z3_ADR <= (others => '1'); 
			INT_VECTOR_CYCLE <='0';
		elsif(falling_edge(FCS))then		

			Z3_ADR(15 downto 2) <= A(15 downto 2);-- latch the whole address for the whole cycle
			
			INT_VECTOR_CYCLE <= not 	MTCR; --is this s special int vector cycle?!?
			
			--use D(15 downto 8)& A(23 downto 16) = A(31 downto 16) for quick response
			
			--lan base
			--if(Z3='1' and (D(15 downto 8) & A(23 downto 16)) = LAN_BASEADR and SHUT_UP_Z2(0)='0' )then	
			if(Z3='1' and (D(15 downto 8) ) = LAN_BASEADR(15 downto 8) and SHUT_UP_Z2(0)='0' )then	--only the upper 8 bits matter!!!
				if(A(14 downto 13)<"11")then
					DQ_SWAP  <= '1';
				else
					DQ_SWAP  <= '0';
				end if;
				LAN_ACCESS 		<= '1';
			else
				LAN_ACCESS 		<= '0';
				DQ_SWAP  <= '1';
			end if;		
		end if;				
	end process ADDRESS_Z3_DECODE;
	
	ADDRESS_Z2_DECODE: process(FCS,reset,AS)
	begin
		if(reset='0' or FCS='1')then
			AUTOCONFIG_Z2_ACCESS <= '0';
			IDE_ACCESS 				<= '0';
			CP_ACCESS				<= '0'; 
		elsif(falling_edge(AS))then		
			
			if(A(23 downto 16) = x"E8" and AUTO_CONFIG_Z2_DONE /= "111" and CFIN='0')then
				AUTOCONFIG_Z2_ACCESS 	<= '1';
			else
				AUTOCONFIG_Z2_ACCESS 	<= '0';
			end if;	

			--CP base
			if(A(23 downto 16) = CP_BASEADR and SHUT_UP_Z2(1)='0' and LAN_ACCESS = '0')then	
				CP_ACCESS 		<= '1';
			else
				CP_ACCESS 		<= '0';
			end if;		

			--IDE base
			if(A(23 downto 16) = IDE_BASEADR and SHUT_UP_Z2(2)='0' and LAN_ACCESS = '0' )then	
				IDE_ACCESS 		<= '1';
			else
				IDE_ACCESS 		<= '0';
			end if;	

		end if;				
	end process ADDRESS_Z2_DECODE;
	
	--LAN interrupt controll
	lan_int_proc: process (CLK_EXT,reset)
	begin
		if(reset ='0') then
			LAN_IRQ_D0 <='1';
			LAN_IRQ_OUT <='1';
		elsif rising_edge(CLK_EXT) then
			LAN_IRQ_D0 <= LAN_INT;
			
			--set/deassert the LAN-interrupt bit
			if(	LAN_INT ='1' --or --no interupt 
					--(LAN_ACCESS = '1' and FCS ='0' and Z3_DS(3) ='0' and RW='0' and 
					--	Z3_ADR(15)='1' and Z3_DATA_IN(30)='1') --deassert via bus controll
				)then
				LAN_IRQ_OUT <='1';
			elsif(
					(LAN_INT ='0' and LAN_IRQ_D0 = '1' ) --or --falling edge
					--(LAN_ACCESS = '1' and FCS ='0' and Z3_DS(3) ='0' and RW='0' and 
					--	Z3_ADR(15)='1' and Z3_DATA_IN(30)='0' and LAN_INT_ENABLE ='1') --set via bus controll
				) then
				LAN_IRQ_OUT <='0';
			end if;
		end if;
	end process lan_int_proc;
	
	--clock this signal to avoid glitches due to short resets
	lan_rst_gen: process (CLK_EXT)
	begin
		if falling_edge(CLK_EXT) then			
			if(FCS ='1' or reset = '0' or Z3_DS = "1111" or LAN_ACCESS = '0'  or INT_VECTOR_CYCLE ='1') then
				LAN_SM_RST <='1';
			else
				LAN_SM_RST <='0';
			end if;
		end if;
	end process lan_rst_gen;
	
	--lan signal generation: all Signals are HIGH active!
	lan_rw_gen: process (CLK_EXT,LAN_SM_RST)
	begin
		if(LAN_SM_RST ='1' ) then
			LAN_SM <=nop;
			LAN_RD_S		<= '0';
			LAN_WRH_S	<= '0';
			LAN_WRL_S	<= '0';
			Z3_A_LOW		<= '0';
			LAN_READY 	<= '0';
			Z3_DATA(31 downto 0) <= x"FFFFFFFF";
			DQ_DATA(15 downto 0) <= x"FFFF";
		elsif rising_edge(CLK_EXT) then			
			--default values
			LAN_RD_S		<= '0';
			LAN_WRH_S	<= '0';
			LAN_WRL_S	<= '0';
			Z3_A_LOW		<= '0';
			LAN_READY 	<= '0';
			case LAN_SM is
				when nop=>
					--cycle start!
				
					-- prepare the data for write
					-- this is a quite complex thing for a cpld 
					-- so I have to move this out of the cycle start condition and prepare it for every loop 
					if(Z3_DS(3 downto 2) < "11")then
						if(DQ_SWAP='0') then
							DQ_DATA(15 downto 0) <= Z3_DATA_IN(31 downto 16);
						else
							DQ_DATA(15 downto 0) <= Z3_DATA_IN(23 downto 16) & Z3_DATA_IN(31 downto 24);
						end if;
					else -- lower word!			
						Z3_A_LOW		<= '1';						
						if(DQ_SWAP='0') then
							DQ_DATA(15 downto 0) <= Z3_DATA_IN(15 downto  0);
						else
							DQ_DATA(15 downto 0) <= Z3_DATA_IN( 7 downto  0) & Z3_DATA_IN(15 downto  8);
						end if;
					end if;	
						
					if(Z3_ADR(15)='1')then
							LAN_SM <= config_rw;
					elsif(RW='1')then --read from MSB			
						if(Z3_DS(3 downto 2) < "11")then --determine bushalf
							LAN_RD_S		<= '1';
							LAN_SM <= wait_read_upper;
						else		
							LAN_RD_S		<= '1';
							LAN_SM <= wait_read_lower;
						end if;
					else
						
						if(Z3_DS(3 downto 2) < "11")then -- determine bushalf
							LAN_SM <= start_write_upper;
						else
							LAN_SM <= start_write_lower;
						end if;
					end if;
				when start_read_upper=>
					LAN_RD_S		<= '1';
					LAN_SM <= wait_read_upper;
				when wait_read_upper=>
					LAN_RD_S		<= '1';
					LAN_SM<=end_read_upper;
				when end_read_upper=>
					--fetch data 
					if(DQ_SWAP='0') then
						Z3_DATA(31 downto 16) <= DQ;
					else
						Z3_DATA(31 downto 16) <= DQ(7 downto 0) & DQ(15 downto 8);
					end if;
					if(Z3_DS(1 downto 0) = "11")then -- no lower half
						LAN_READY <='1';
						LAN_SM <= end_read_upper;  -- stay here until cylce end
					else
						Z3_A_LOW		<= '1';
						LAN_SM <= start_read_lower;
					end if;
				when start_read_lower=>
					Z3_A_LOW		<= '1';
					LAN_RD_S		<= '1';
					LAN_SM <= wait_read_lower;
				when wait_read_lower=>
					Z3_A_LOW		<= '1';
					LAN_RD_S		<= '1';
					LAN_READY 	<= '1';
					LAN_SM<=end_read_lower;
				when end_read_lower=>
					--fetch data 
					if(DQ_SWAP='0') then
						Z3_DATA(15 downto 0) <= DQ;
					else
						Z3_DATA(15 downto 0) <= DQ(7 downto 0) & DQ(15 downto 8);
					end if;
					LAN_READY <='1';
					LAN_SM<=end_read_lower; -- stay here until cylce end
				when start_write_upper=>
					-- swapped DS3/DS2 here: ENC624 is little endian
					LAN_WRH_S   <= not Z3_DS(2);
					LAN_WRL_S   <= not Z3_DS(3);
					LAN_SM <= wait_write_upper;
				when wait_write_upper=>
					LAN_SM<=end_write_upper;
				when end_write_upper=>
					-- prepare the data for write
					if(DQ_SWAP='0') then
						DQ_DATA(15 downto 0) <= Z3_DATA_IN(15 downto  0);
					else
						DQ_DATA(15 downto 0) <= Z3_DATA_IN( 7 downto  0) & Z3_DATA_IN(15 downto  8);
					end if;
									
					if(Z3_DS(1 downto 0) = "11")then -- no lower half
						LAN_READY <='1';
						LAN_SM <= end_write_upper;  -- stay here until cylce end
					else
						Z3_A_LOW		<= '1';
						LAN_SM <= start_write_lower;
					end if;
				when start_write_lower=>
					Z3_A_LOW		<= '1';
					-- swapped DS0/DS1 here: ENC624 is little endian
					LAN_WRH_S   <= not Z3_DS(0);
					LAN_WRL_S   <= not Z3_DS(1);
					LAN_SM <= wait_write_lower;
				when wait_write_lower=>
					Z3_A_LOW		<= '1';
					LAN_READY <='1';
					LAN_SM<=end_write_lower;
				when end_write_lower=>
					LAN_READY <='1';
					LAN_SM<=end_write_lower; -- stay here until cylce end
				when config_rw=>
					LAN_READY <='1';
					if(Z3_DS(3) ='0' and RW='0')then 
						LAN_INT_ENABLE <= Z3_DATA_IN(31); --this controlls the output bit
					elsif(Z3_DS(1) ='0' and RW='0')then
						LAN_INT_ENABLE <= Z3_DATA_IN(15); --this controlls the output bit
					end if;
					LAN_SM<=config_rw; -- stay here until cylce end
				end case;			
		end if;
	end process lan_rw_gen;

	--autoconfig	
	autoconfig_proc: process (reset, AMIGA_CLK)
	begin
		if	reset = '0' then
			-- reset active ...
			AUTO_CONFIG_Z2_DONE_CYCLE	<="000";
			D_Z2_OUT<="1111";
			SHUT_UP_Z2	<="111";
			IDE_BASEADR<=x"FF";
			CP_BASEADR<=x"FF";
			AUTO_CONFIG_Z2_DONE	<="000";
			LAN_BASEADR<=x"FFFF";
		elsif falling_edge(AMIGA_CLK) then -- no reset, so wait for rising edge of the clock		
			D_Z2_OUT<="1111";
			if(AS='1')then
				AUTO_CONFIG_Z2_DONE <= AUTO_CONFIG_Z2_DONE or AUTO_CONFIG_Z2_DONE_CYCLE;
			elsif(AUTOCONFIG_Z2_ACCESS= '1' and DS='0') then
				case A(6 downto 1) is
					when "000000"	=> 
						if(AUTO_CONFIG_Z2_DONE(0) = '0')then
							D_Z2_OUT <= 	"1000" ; --ZII, No-System-Memory, no ROM
						elsif(AUTO_CONFIG_Z2_DONE(1) = '0')then
							D_Z2_OUT <= 	"1100" ; --ZII, No-System-Memory, no ROM
						else
							D_Z2_OUT <= 	"1101" ; --ZII, no System-Memory, (perhaps)ROM
						end if;
					when "000001"	=> D_Z2_OUT <=	"0001" ; --one Card, 64KB =001
					when "000010"	=> 
						if(AUTO_CONFIG_Z2_DONE(0) = '0' or AUTO_CONFIG_Z2_DONE(1) = '0')then
							D_Z2_OUT <=	"1000" ; --ProductID high nibble : 7->1000
						else
							D_Z2_OUT <=	"1111" ; --ProductID high nibble : 0->1111
						end if;
					when "000011"	=> 
						if(AUTO_CONFIG_Z2_DONE(0) = '0')then
							D_Z2_OUT <=	"0100" ; --ProductID low nibble: B->0100
						elsif(AUTO_CONFIG_Z2_DONE(1) = '0')then
							D_Z2_OUT <=	"0011" ; --ProductID low nibble: C->0011
						else
							D_Z2_OUT <=	"1001" ; --ProductID low nibble: 6->1001 
						end if;						
					when "000100"	=> --er_flags (Z3) 
						D_Z2_OUT <=	"1110" ; -- io device no size extension 64kb subsize    	
					when "000101"	=> --er_flags (Z3) 
						D_Z2_OUT <=	"1101" ; -- io device no size extension 64kb subsize    	
					when "001000"	=> D_Z2_OUT <=	"1111" ; --Ventor ID 0
					
					when "001001"	=> 
						if(AUTO_CONFIG_Z2_DONE(0) = '0' or AUTO_CONFIG_Z2_DONE(1) = '0')then
							D_Z2_OUT <=	"0101" ; --Ventor ID 1
						else
							D_Z2_OUT <=	"0111" ; --Ventor ID 1
						end if;						
					when "001010"	=> 
						if(AUTO_CONFIG_Z2_DONE(0) = '0' or AUTO_CONFIG_Z2_DONE(1) = '0')then
							D_Z2_OUT <=	"1110" ; --Ventor ID 2
						else
							D_Z2_OUT <=	"1101" ; --Ventor ID 2
						end if;												
					when "001011"	=> 
						if(AUTO_CONFIG_Z2_DONE(0) = '0' or AUTO_CONFIG_Z2_DONE(1) = '0')then
							D_Z2_OUT <=	"0011" ; --Ventor ID 3 : $0A1C: A1K.org
						else
							D_Z2_OUT <=	"0011" ; --Ventor ID 3 : $082C: BSC
						end if;						
					when "001100"	=> D_Z2_OUT <=	"0100" ; --Serial byte 0 (msb) high nibble
					when "001101"	=> D_Z2_OUT <=	"1110" ; --Serial byte 0 (msb) low  nibble
					when "001110"	=> D_Z2_OUT <=	"1001" ; --Serial byte 1       high nibble
					when "001111"	=> D_Z2_OUT <=	"0100" ; --Serial byte 1       low  nibble
					when "010000"	=> D_Z2_OUT <=	"1111" ; --Serial byte 2       high nibble
					when "010001"	=> D_Z2_OUT <=	"1111" ; --Serial byte 2       low  nibble
					when "010010"	=> D_Z2_OUT <=	"0100" ; --Serial byte 3 (lsb) high nibble
					when "010011"	=> D_Z2_OUT <=	"1010" ; --Serial byte 3 (lsb) low  nibble: B16B00B5
					--when "010100"	=> Dout1 <=	"1111" ; --Rom vector high byte high nibble 
					--when "010101"	=> Dout1 <=	"1111" ; --Rom vector high byte low  nibble 
					--when "010110"	=> Dout1 <=	"1111" ; --Rom vector low byte high nibble
					when "010111"	=> 
						D_Z2_OUT <=	"1110" ; --Rom vector low byte low  nibble						
					when "100010"	=>
						D_Z2_OUT <=	"1111" ;
						if(RW='0')then
							if(AUTO_CONFIG_Z2_DONE(0) = '0')then
								LAN_BASEADR(15 downto 0)	<= D(15 downto 0); --Base adress
								SHUT_UP_Z2(0)					<='0'; --enable board
								AUTO_CONFIG_Z2_DONE_CYCLE(0)	<= '1'; --done here
							end if;
						end if;	
					when "100100"	=>
						D_Z2_OUT <=	"1111" ;
						if(RW='0' and AUTO_CONFIG_Z2_DONE(0)='1')then
							if(AUTO_CONFIG_Z2_DONE(1) = '0')then
								CP_BASEADR(7 downto 0)	<= D(15 downto 8); --Base adress
								SHUT_UP_Z2(1)					<='0'; --enable board
								AUTO_CONFIG_Z2_DONE_CYCLE(1)	<= '1'; --done here
							elsif(AUTO_CONFIG_Z2_DONE(2) = '0')then									
								IDE_BASEADR(7 downto 0)	<= D(15 downto 8); --Base adress
								SHUT_UP_Z2(2)					<= '0'; --enable board
								AUTO_CONFIG_Z2_DONE_CYCLE(2)	<= '1'; --done here
							end if;
						end if;
					when "100110"	=>
						D_Z2_OUT <=	"1111" ;
						if(RW='0')then
							if(AUTO_CONFIG_Z2_DONE(0) = '0')then
								AUTO_CONFIG_Z2_DONE_CYCLE(0)	<= '1'; --done here
							elsif(AUTO_CONFIG_Z2_DONE(1) = '0')then									
								AUTO_CONFIG_Z2_DONE_CYCLE(1)	<= '1'; --done here
							elsif(AUTO_CONFIG_Z2_DONE(2) = '0')then									
								AUTO_CONFIG_Z2_DONE_CYCLE(2)	<= '1'; --done here	
							end if;
						end if;
					when others	=> D_Z2_OUT <=	"1111" ;
				end case;				
			end if;
		end if;
	end process autoconfig_proc; --- that's all



	LAN_CS	<= LAN_ACCESS   when reset='1' and Z3_ADR(15)='0' else LAN_CS_RST;						
	LAN_WRL	<= LAN_WRL_S when FCS='0' and reset = '1' else LAN_WR_RST;
	LAN_WRH	<= LAN_WRH_S when FCS='0' and reset = '1' else LAN_WR_RST;
	LAN_RD	<= LAN_RD_S  when FCS='0' and reset = '1' else '0';
	LAN_CFG	<= "ZZZZ";
	

	A_LAN(13 downto 6)<=	 LAN_A_INIT(13 downto 6) when reset ='0' else
								 Z3_ADR(14 downto 7); 	
	A_LAN(5 downto 2) <=	 LAN_A_INIT(5 downto 2) when reset ='0' else
								 A(5 downto 2) when CP_ACCESS='1' else --mux the clock-port adresses! 
								 Z3_ADR(6 downto 3); 
	A_LAN(1 downto 0)<=	 LAN_A_INIT(1 downto 0) when reset ='0' else
								 Z3_ADR(2) & Z3_A_LOW; 
	
	
	
	--signal assignment
	D(15 downto 0)	<=	Z3_DATA(31 downto 16) 	when RW='1' and FCS='0' and LAN_ACCESS ='1' else		
							D_Z2_OUT	& x"FFF" 		when RW='1' and  AS='0' and AUTOCONFIG_Z2_ACCESS ='1' else
							DQ(7 downto 0)&DQ(7 downto 0)  when RW='1' and CP_ACCESS = '1'     and AS='0' else
							(others => 'Z');

	A(23 downto 8)	<=	Z3_DATA(15 downto  0) 	when RW='1' and FCS='0' and LAN_ACCESS ='1' else			
							(others => 'Z');
			
	A(7 downto 1) <= (others => 'Z');

	--defined lancp signal that matches both LAN and CP addresses
	DQ <=	LAN_D_INIT 					when reset='0' else
			DQ_DATA(15 downto  0)	when RW='0' and FCS='0' and Z3_DS <"1111" and LAN_ACCESS ='1' else
			D 	when RW='0' and AS='0' and CP_ACCESS ='1' else
			(others => 'Z');





	INT2_OUT <= '0' when LAN_IRQ_OUT = '0' and LAN_INT_ENABLE = '1' else
					--'0' when CP_IRQ = '0' else
				  'Z';
	INT6_OUT <= '0' when CP_IRQ = '0' else
				  'Z';
	OWN 	<= 'Z';
	SLAVE <= '0' when FCS='0' and LAN_ACCESS = '1' else 
				'0' when  AS='0' and (AUTOCONFIG_Z2_ACCESS  = '1' or IDE_ACCESS = '1' or CP_ACCESS = '1') else '1';	
	CFOUT <= '0' when AUTO_CONFIG_Z2_DONE ="111" else '1';
	
	OVR <= '0' when FCS='0' and LAN_ACCESS = '1' else 'Z'; --is Cache inhibit on Z3!

	DTACK <= '0' when FCS='0' and LAN_READY = '1' else 'Z';

   MTACK <= 'Z';


	--ide signal generation
	ide_rw_gen: process (reset,AMIGA_CLK)
	begin
		if	(reset = '0') then
			IDE_ENABLE 	<= '1';
			IDE_R_S		<= '1';
			IDE_W_S		<= '1';
			ROM_OE_S		<= '1';
		elsif falling_edge(AMIGA_CLK) then			
			--default values
			IDE_R_S		<= '1';
			IDE_W_S		<= '1';
			ROM_OE_S		<= '1';					
			if(IDE_ACCESS='1' and AS='0')then
				if(RW='0')then
					--the write goes to the hdd!
					IDE_ENABLE  <= '0'; -- enable IDE on first read
					IDE_W_S		<= '0';	
				else
					IDE_R_S		<= IDE_ENABLE; --read from IDE instead from ROM
					ROM_OE_S		<=	not IDE_ENABLE;						
				end if;	
			end if;				
		end if;
	end process ide_rw_gen;

	IDE_W <= IDE_W_S	when AS='0' else '1';
	IDE_R <=	IDE_R_S	when AS='0' else '1';
	ROM_OE<= ROM_OE_S	when AS='0' and AUTOBOOT_OFF ='0' else '1';			
	IDE_CS(0)<= not(A(12));			
	IDE_CS(1)<= not(A(13));
	IDE_A(2 downto 0)	<= A(11 downto 9);
	ROM_B	<= "00";


	--cp signal generation
	cp_rw_gen: process (AMIGA_CLK)
	begin
		if falling_edge(AMIGA_CLK) then			
			--default values
			CP_RD_S		<= '1';
			CP_WE_S		<= '1';
			if(CP_ACCESS = '1' and DS='0')then --datastrobe instead of AS!
				CP_RD_S		<= not RW;
				CP_WE_S		<= RW;
			end if;				
			CP_WE_QUIRK <= CP_WE_S; --the clockport write must be low exactly one 7MHz cycle!
		end if;
	end process cp_rw_gen;
	
	--for the future
	CP_WE		<= CP_WE_S when AS='0' and CP_WE_QUIRK ='1' else '1';
	CP_RD		<= CP_RD_S when AS='0' else '1';
	CP_CS		<= not CP_ACCESS;

end Behavioral;

