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
           INT_OUT : out  STD_LOGIC;
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
				end_write_lower
	);

	SIGNAL LAN_RST_SM: lan_reset :=nop;
	SIGNAL LAN_SM: lan_bus_sm :=nop;
	SIGNAL autoconfig: STD_LOGIC;
	SIGNAL lan_adr: STD_LOGIC;
	SIGNAL dq_swap: STD_LOGIC;
	signal Dout1:STD_LOGIC_VECTOR(7 downto 0):=x"FF";
	signal AUTO_CONFIG_DONE:STD_LOGIC;
	signal AUTO_CONFIG_DONE_CYCLE:STD_LOGIC;
	signal SHUT_UP:STD_LOGIC;
	signal LAN_BASEADR:STD_LOGIC_VECTOR(15 downto 0);
	signal LAN_INT_ENABLE: std_logic;
	signal DECODE_RESET: std_logic;
	signal LAN_RD_S: std_logic;
	signal LAN_WRH_S: std_logic;
	signal LAN_WRL_S: std_logic;
	signal lan_rdy: std_logic;
	signal LAN_IRQ_D0: std_logic;
	signal LAN_IRQ_OUT: std_logic;
	signal Z3_ADR:STD_LOGIC_VECTOR(15 downto 2);
	signal Z3_DATA_IN:STD_LOGIC_VECTOR(31 downto 0);
	signal Z3_DATA:STD_LOGIC_VECTOR(31 downto 0);
	signal DQ_DATA:STD_LOGIC_VECTOR(15 downto 0);
	signal Z3_DS:STD_LOGIC;
	signal Z3_A_LOW:STD_LOGIC;
	signal LAN_SM_RST:STD_LOGIC;
	
	signal LAN_CS_RST: std_logic;
	signal LAN_WR_RST: std_logic;
	
	signal LAN_A_INIT:STD_LOGIC_VECTOR(13 downto 0) :="11"&x"FFF";
	signal LAN_D_INIT:STD_LOGIC_VECTOR(15 downto 0) := x"0000";
	constant LAN_A_CLRREG:STD_LOGIC_VECTOR(13 downto 0) :="11"&x"FF7";
	constant LAN_A_SETREG:STD_LOGIC_VECTOR(13 downto 0) :="11"&x"FB7";
	--constant LAN_D_SET:STD_LOGIC_VECTOR(15 downto 0) := "0000000100010000"; --33MHz
	--constant LAN_D_CLR:STD_LOGIC_VECTOR(15 downto 0) := "0000111000000000"; --33MHz
	constant LAN_D_SET:STD_LOGIC_VECTOR(15 downto 0) :=   "0000001000010010"; --25MHz	
	constant LAN_D_CLR:STD_LOGIC_VECTOR(15 downto 0) :=   "0000110100000000"; --25MHz
	
   Function to_std_logic(X: in Boolean) return Std_Logic is
   variable ret : std_logic;
   begin
   if x then ret := '1';  else ret := '0'; end if;
   return ret;
   end to_std_logic;
	
begin

	Z3_DATA_IN <= D(15 downto 0) & A(23 downto 8);
	DECODE_RESET <= BERR and reset;
	
	Z3_DS <= UDS and LDS and DS1 and DS0;
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
	
	ADDRESS_DECODE: process(DECODE_RESET,FCS)
	begin
		if(DECODE_RESET ='0')then
			autoconfig 	<= '0';
			lan_adr 		<= '0';
			dq_swap  <= '0';
			Z3_ADR <= (others => '1'); 
		elsif(falling_edge(FCS))then		
			--default values
			autoconfig 	<= '0';
			lan_adr 		<= '0';
			dq_swap  <= '0';
			Z3_ADR(15 downto 2) <= A(15 downto 2);-- latch the whole address for the whole cycle
			
			--use D(15 downto 8)& A(23 downto 16) = A(31 downto 16) for quick response
			--autoconfig
			if(Z3='1' and (D(15 downto 8)& A(23 downto 16)) = x"FF00" and AUTO_CONFIG_DONE = '0' and CFIN='0')then
				autoconfig 	<= '1';
			end if;
			
			--lan base
			if(Z3='1' and (D(15 downto 8) & A(23 downto 16)) = LAN_BASEADR and SHUT_UP='0' )then	
				if(A(14 downto 13)<"11")then
					dq_swap  <= '1';
				end if;
				lan_adr 		<= '1';
			end if;		
		end if;				
	end process ADDRESS_DECODE;
	
	--LAN interrupt enable
	lan_int_proc: process (CLK_EXT,reset)
	begin
		if(reset ='0') then
			LAN_INT_ENABLE <='0';
			LAN_IRQ_D0 <='1';
			LAN_IRQ_OUT <='1';
		elsif rising_edge(CLK_EXT) then
			LAN_IRQ_D0 <= LAN_INT;
			if(LAN_INT ='0' and LAN_IRQ_D0 ='1') then
				LAN_IRQ_OUT <='0';
			elsif(LAN_INT ='1' or LAN_INT_ENABLE = '0')then
				LAN_IRQ_OUT <='1';
			elsif(lan_adr = '1' and Z3_DS ='0' and RW='0' and Z3_ADR(15)='1') then 
				LAN_IRQ_OUT		<= Z3_DATA_IN(30); --controll lan irq
			end if;
			if(lan_adr = '1' and Z3_DS ='0' and RW='0' and Z3_ADR(15)='1') then --enable if a write to A15 occured
				LAN_INT_ENABLE <= Z3_DATA_IN(31);
			end if;
		end if;
	end process lan_int_proc;
	
	--clock this signal to avoid glitches due to short resets
	lan_rst_gen: process (CLK_EXT)
	begin
		if rising_edge(CLK_EXT) then			
			if(FCS ='1' or reset = '0') then
				LAN_SM_RST <='1';
			else
				LAN_SM_RST <='0';
			end if;
		end if;
	end process lan_rst_gen;
	
	--lan signal generation: all Signals are HIGH active!
	lan_rw_gen: process (CLK_EXT,FCS,reset)
	begin
		if(LAN_SM_RST ='1' ) then
			LAN_SM <=nop;
			LAN_RD_S		<= '0';
			LAN_WRH_S	<= '0';
			LAN_WRL_S	<= '0';
			Z3_A_LOW		<= '0';
			lan_rdy <='0';
			Z3_DATA(31 downto 0) <= x"FFFFFFFF";
			DQ_DATA(15 downto 0) <= x"FFFF";
		elsif rising_edge(CLK_EXT) then			
			--default values
			LAN_RD_S		<= '0';
			LAN_WRH_S	<= '0';
			LAN_WRL_S	<= '0';
			Z3_A_LOW		<= '0';
			lan_rdy <='0';
			case LAN_SM is
				when nop=>
				
				
					-- prepare the data for write
					-- this is a quite complex thing for a cpld 
					-- so I have to move this out of the cycle start condition and prepare it for every loop 
					if(UDS='0' or LDS='0')then
						if(dq_swap='0') then
							DQ_DATA(15 downto 0) <= Z3_DATA_IN(31 downto 16);
						else
							DQ_DATA(15 downto 0) <= Z3_DATA_IN(23 downto 16) & Z3_DATA_IN(31 downto 24);
						end if;	
					else
						if(dq_swap='0') then
							DQ_DATA(15 downto 0) <= Z3_DATA_IN(15 downto  0);
						else
							DQ_DATA(15 downto 0) <= Z3_DATA_IN( 7 downto  0) & Z3_DATA_IN(15 downto  8);
						end if;
					end if;
					if(lan_adr = '1' and Z3_DS = '0' and Z3_ADR(15) = '0' )then --cycle start!
						
						if(RW='1')then --read from MSB
							-- determine bushalf
							if(UDS='0' or LDS='0')then
								LAN_SM <= wait_read_upper;
								LAN_RD_S		<= '1';
							else
								Z3_A_LOW		<= '1';
								LAN_RD_S		<= '1';
								LAN_SM <= wait_read_lower;
							end if;
						else
					
							-- determine bushalf
							if(UDS='0' or LDS='0')then
								-- prepare the data for write
								if(dq_swap='0') then
									DQ_DATA(15 downto 0) <= Z3_DATA_IN(31 downto 16);
								else
									DQ_DATA(15 downto 0) <= Z3_DATA_IN(23 downto 16) & Z3_DATA_IN(31 downto 24);
								end if;	
								LAN_SM <= start_write_upper;
							else
								-- prepare the data for write
								if(dq_swap='0') then
									DQ_DATA(15 downto 0) <= Z3_DATA_IN(15 downto  0);
								else
									DQ_DATA(15 downto 0) <= Z3_DATA_IN( 7 downto  0) & Z3_DATA_IN(15 downto  8);
								end if;
								Z3_A_LOW		<= '1';
								LAN_SM <= start_write_lower;
							end if;
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
					if(dq_swap='0') then
						Z3_DATA(31 downto 16) <= DQ;
					else
						Z3_DATA(31 downto 16) <= DQ(7 downto 0) & DQ(15 downto 8);
					end if;
					if(DS1='1' and DS0='1')then -- no lower half
						lan_rdy <='1';
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
					LAN_SM<=end_read_lower;
				when end_read_lower=>
					--fetch data 
					if(dq_swap='0') then
						Z3_DATA(15 downto 0) <= DQ;
					else
						Z3_DATA(15 downto 0) <= DQ(7 downto 0) & DQ(15 downto 8);
					end if;
					lan_rdy <='1';
					LAN_SM<=end_read_lower; -- stay here until cylce end
				when start_write_upper=>
					-- swapped LDS/UDS here: ENC624 is little endian
					LAN_WRH_S   <= not LDS;
					LAN_WRL_S   <= not UDS;
					LAN_SM <= wait_write_upper;
				when wait_write_upper=>
					-- swapped LDS/UDS here: ENC624 is little endian
					LAN_WRH_S   <= not LDS;
					LAN_WRL_S   <= not UDS;
					LAN_SM<=end_write_upper;
				when end_write_upper=>
					-- prepare the data for write
					if(dq_swap='0') then
						DQ_DATA(15 downto 0) <= Z3_DATA_IN(15 downto 0);
					else
						DQ_DATA(15 downto 0) <= Z3_DATA_IN(7 downto 0) & Z3_DATA_IN(15 downto 8);
					end if;
				
					if(DS1='1' and DS0='1')then -- no lower half
						lan_rdy <='1';
						LAN_SM <= end_write_upper;  -- stay here until cylce end
					else
						Z3_A_LOW		<= '1';
						LAN_SM <= start_write_lower;
					end if;
				when start_write_lower=>
					Z3_A_LOW		<= '1';
					-- swapped DS0/DS1 here: ENC624 is little endian
					LAN_WRH_S   <= not DS0;
					LAN_WRL_S   <= not DS1;
					LAN_SM <= wait_write_lower;
				when wait_write_lower=>
					Z3_A_LOW		<= '1';
					-- swapped DS0/DS1 here: ENC624 is little endian
					LAN_WRH_S   <= not DS0;
					LAN_WRL_S   <= not DS1;
					LAN_SM<=end_write_lower;
				when end_write_lower=>
					lan_rdy <='1';
					LAN_SM<=end_write_lower; -- stay here until cylce end
			end case;			
		end if;
	end process lan_rw_gen;
		
	
	--autoconfig	
	autoconfig_proc: process (reset, CLK_EXT)
	begin
		if	reset = '0' then
			-- reset active ...
			AUTO_CONFIG_DONE_CYCLE	<='0';
			Dout1<=x"FF";
			SHUT_UP	<='1';
			LAN_BASEADR<=x"FFFF";
			AUTO_CONFIG_DONE	<='0';
		elsif rising_edge(CLK_EXT) then -- no reset, so wait for rising edge of the clock		
			--default value
			Dout1<=x"FF";
			if(FCS='1')then
				AUTO_CONFIG_DONE <= AUTO_CONFIG_DONE_CYCLE or not AUTOBOOT_OFF ;
			elsif(autoconfig= '1' and Z3_DS='0' ) then		
				case Z3_ADR(8 downto 2) is
					when "0000000"	=> Dout1 <= "10000001" ; --Z3, No mem, no Rom, single, board, 64kb 
					when "1000000"	=> Dout1 <= "00010001" ; --Z3, No mem, no Rom, single, board, 64kb 
					when "0000001"	=> Dout1 <=	"10000100" ; --ProductID: 7B->10000100
					when "1000001"	=> Dout1 <=	"01000100" ; --ProductID: 7B->10000100
					when "0000010"	=> Dout1 <=	"11101101" ; --Flags
					when "1000010"	=> Dout1 <=	"11011101" ; --Flags
					when "0000100"	=> Dout1 <=	"11110101" ; --Ventor ID 0/1
					when "1000100"	=> Dout1 <=	"01010101" ; --Ventor ID 0/1
					when "0000101"	=> Dout1 <=	"11100011" ; --Ventor ID 2/3 : $0A1C: A1K.org
					when "1000101"	=> Dout1 <=	"00110011" ; --Ventor ID 2/3 : $0A1C: A1K.org
					when "0010010"	=>
						if(RW='0')then
							LAN_BASEADR(15 downto 0)	<= Z3_DATA_IN(31 downto 16); --Base address
							SHUT_UP					<='0'; --enable board
							AUTO_CONFIG_DONE_CYCLE	<= '1'; --done here
						end if;
					when "0010011"	=>
						if(RW='0')then
							AUTO_CONFIG_DONE_CYCLE	<= '1'; --done here
						end if;
					when others =>
						--nothing
				end case;	
			end if;
		end if;
	end process autoconfig_proc; --- that's all

	LAN_CS	<= lan_adr   when reset='1' else LAN_CS_RST;						
	LAN_WRL	<= LAN_WRL_S when FCS='0' and reset = '1' else LAN_WR_RST;
	LAN_WRH	<= LAN_WRH_S when FCS='0' and reset = '1' else LAN_WR_RST;
	LAN_RD	<= LAN_RD_S  when FCS='0' and reset = '1' else '0';
	LAN_CFG	<= "ZZZZ";
	

	A_LAN(13 downto 0)<=	 LAN_A_INIT when reset ='0' else
								 Z3_ADR(14 downto 2) & Z3_A_LOW; 
	
	--signal assignment
	D(15 downto 0)	<=	Z3_DATA(31 downto 16) 	when RW='1' and Z3_DS ='0' and FCS='0' and lan_adr ='1' else		
							Dout1	& x"FF" 				when RW='1' and Z3_DS ='0' and FCS='0' and autoconfig ='1' else
							(others => 'Z');

	A(23 downto 8)	<=	Z3_DATA(15 downto  0) 	when RW='1' and Z3_DS ='0' and FCS='0' and lan_adr ='1' else			
							(others => 'Z');
			
	A(7 downto 1) <= (others => 'Z');

	--defined lancp signal that matches both LAN and CP addresses
	DQ <=	LAN_D_INIT 					when reset='0' else
			DQ_DATA(15 downto  0)	when RW='0' and FCS='0' and Z3_DS ='0' and lan_adr ='1' else
			(others => 'Z');

	INT_OUT <= '0' when LAN_IRQ_OUT = '0' and LAN_INT_ENABLE = '1' else
				  'Z';
	
	OWN 	<= 'Z';
	SLAVE <= '0' when FCS='0' and (autoconfig  = '1' or lan_adr = '1') else '1';	
	CFOUT <= '0' when AUTO_CONFIG_DONE='1' else '1';
	
	OVR <= 'Z';

	DTACK <= '0' when FCS='0' and (lan_rdy = '1' or autoconfig ='1') else 'Z';

   MTACK <= 'Z';


	--for the future
	CP_WE		<= '1';
	CP_RD		<= '1';
	CP_CS		<= '1';
	IDE_W <=	'1';
	IDE_R <=	'1';
	IDE_CS(0)<= '1';			
	IDE_CS(1)<= '1';
	IDE_A(2 downto 0)	<= "111";
	ROM_B	<= "11";
	ROM_OE	<= '1';				


end Behavioral;

