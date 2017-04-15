-- 1D IDCT
  --
  -- Computes the 1D IDCT with 8bits color components
  -- This implementation performs all 12 multiplications in parallel, 
  -- and thus computes the output values in one clock cycle
  --
  -- Port usage:
  ---- i0, ..., i7: input coefficients, signed 16 bits
  ---- o0, ..., o7: output coefficients, signed 16 bits
  ---- pass: determine the pass: 0 = Pass1, 1 = Pass2
  --
  -- See Section 4.3 in the Lab Manual for a detailed explanation
  -- of how the 1D IDCT is used to compute the 2D IDCT, and how
  -- the bits scaling is performed between passes.

  library ieee;
  use ieee.std_logic_1164.all;
  use ieee.numeric_std.all;
  use ieee.std_logic_arith.all;
  use ieee.std_logic_unsigned.all;


  entity twod_idct_st_hw is
    PORT(
         CLOCK : IN STD_LOGIC;
         DATAOUT : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
         DST_READY : IN STD_LOGIC;
         DST_VALID : OUT STD_LOGIC:='0';
         RESET : IN STD_LOGIC;
         DATAIN : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
         SRC_READY : OUT STD_LOGIC:='1';
         SRC_VALID : IN STD_LOGIC
         );
  end entity twod_idct_st_hw;

  architecture a_idct_2D of twod_idct_st_hw is

  -- DUT component
  component idct_1D is
  port(
    signal clk: in std_logic;       -- CPU system clock (always required)

    signal pass : in std_logic;     -- 0: Pass 1; 1: Pass 2

    signal i0: in std_logic_vector(15 downto 0);
    signal i1: in std_logic_vector(15 downto 0);
    signal i2: in std_logic_vector(15 downto 0);
    signal i3: in std_logic_vector(15 downto 0);
    signal i4: in std_logic_vector(15 downto 0);
    signal i5: in std_logic_vector(15 downto 0);
    signal i6: in std_logic_vector(15 downto 0);
    signal i7: in std_logic_vector(15 downto 0);

    signal o0: out std_logic_vector(15 downto 0);
    signal o1: out std_logic_vector(15 downto 0);
    signal o2: out std_logic_vector(15 downto 0);
    signal o3: out std_logic_vector(15 downto 0);
    signal o4: out std_logic_vector(15 downto 0);
    signal o5: out std_logic_vector(15 downto 0);
    signal o6: out std_logic_vector(15 downto 0);
    signal o7: out std_logic_vector(15 downto 0)
      );
  end component;

    TYPE BUF IS ARRAY (63 DOWNTO 0) OF STD_LOGIC_VECTOR(15 DOWNTO 0);
    signal INBUF: BUF;
    signal TMPBUF: BUF;
    signal OUTBUF: BUF;

   signal reset_triggered: std_logic;
    signal TMPBUF_FREE : std_logic:='1';
    signal OUTBUF_FREE : std_logic:='1';
    signal SRC_READY_SIG : std_logic:='1';
    signal DST_VALID_SIG : std_logic:='0';
    signal LAST_DST_VALID_SIG : std_logic:='0';
    signal catch_up_data : std_logic:='0';

    signal sweep_horizontal_count : integer range 0 to 9 := 0;
    signal IDCT_count : integer range 0 to 9 := 0;
    signal DATA_count : integer range 0 to 64 := 0;
    signal DATA_input_count : integer range 0 to 64 := 0;
    signal IDCT_timeout : integer range 0 to 4 := 0;
    signal TMPBUF_count : integer range 0 to 9 := 0;
    signal IDCT_flag : integer range 0 to 8 := 0;
    signal dma_output_count : integer range 0 to 33 := 0;
    signal current_dma_output_count : integer range 0 to 33 := 0;
    signal fetch_outbuf : integer range 0 to 8 := 0;
    signal fetch_finbuf : integer range 0 to 8 := 0;
    
    signal inbuf0:  std_logic_vector(15 downto 0);
    signal inbuf1:  std_logic_vector(15 downto 0);
    signal outbuf0:  std_logic_vector(63 downto 0);
    signal outbuf1:  std_logic_vector(63 downto 0);
    signal finbuf0:  std_logic_vector(63 downto 0);
    signal finbuf1:  std_logic_vector(63 downto 0);
    signal fin4realbuf0:  std_logic_vector(63 downto 0);
    signal fin4realbuf1:  std_logic_vector(63 downto 0);
    
           signal LAST_DST_READY_STATE : std_logic:='0';
           
       signal testtest : std_logic:='0';


      signal i0_r:  std_logic_vector(15 downto 0);
        signal i1_r:  std_logic_vector(15 downto 0);
        signal i2_r:  std_logic_vector(15 downto 0);
        signal i3_r:  std_logic_vector(15 downto 0);
        signal i4_r:  std_logic_vector(15 downto 0);
        signal i5_r:  std_logic_vector(15 downto 0);
        signal i6_r:  std_logic_vector(15 downto 0);
        signal i7_r:  std_logic_vector(15 downto 0);

        signal o0_r:  std_logic_vector(15 downto 0);
        signal o1_r:  std_logic_vector(15 downto 0);
        signal o2_r:  std_logic_vector(15 downto 0);
        signal o3_r:  std_logic_vector(15 downto 0);
        signal o4_r:  std_logic_vector(15 downto 0);
        signal o5_r:  std_logic_vector(15 downto 0);
        signal o6_r:  std_logic_vector(15 downto 0);
        signal o7_r:  std_logic_vector(15 downto 0);

      signal PASS_R :  std_logic:='0';
  begin



    IDCT_1d_i: idct_1D

    port map(
        clk => CLOCK,     -- CPU system clock (always required)
        pass => PASS_R,   -- 0: Pass 1; 1: Pass 2

        i0 => i0_r,
        i1 => i1_r,
        i2 => i2_r,
        i3 => i3_r,
        i4 => i4_r,
        i5 => i5_r,
        i6 => i6_r,
        i7 => i7_r,


        o0 => o0_r,
        o1 => o1_r,
        o2 => o2_r,
        o3 => o3_r,
        o4 => o4_r,
        o5 => o5_r,
        o6 => o6_r,
        o7 => o7_r
    );

    -------------------------------------------------------------
    process begin
    
    
    
    
    --todo senstivvity list clk reset

    wait until rising_edge(CLOCK);
    reset_triggered <= RESET;
   
    if (RESET='1')THEN
    TMPBUF_FREE<='1';
    OUTBUF_FREE<='1';
    SRC_READY_SIG<='1';
    SRC_READY <='1';
    DST_VALID <='0';
    DST_VALID_SIG<='0';
    sweep_horizontal_count <=0;
    IDCT_count <=0;
    DATA_count<=0;
    DATA_input_count <=0;
    IDCT_timeout <=0;
    TMPBUF_count<=0;
    IDCT_flag<=0;
    dma_output_count <=0;
    current_dma_output_count<=0;
    fetch_outbuf<=0;
    fetch_finbuf<=0;
    END IF;

    
      if (SRC_VALID='1' and sweep_horizontal_count=0 and SRC_READY_SIG='1' and TMPBUF_FREE='1') THEN
      
      
        INBUF(DATA_input_count*2+1) <= DATAIN (7 DOWNTO 0) &DATAIN (15 DOWNTO 8);
        INBUF(DATA_input_count*2) <= DATAIN (23 DOWNTO 16) & DATAIN (31 DOWNTO 24);

     INBUF1 <= DATAIN (7 DOWNTO 0) &DATAIN (15 DOWNTO 8);
       INBUF0 <= DATAIN (23 DOWNTO 16) & DATAIN (31 DOWNTO 24);
      -- counter for TOTAL number of bytes sent in this block (max 64)
        DATA_input_count <= DATA_input_count + 1;
      
        -- counter for number of bytes in this IDCT Conv (max 4)
        if (DATA_count < 4 ) THEN
        DATA_count <= DATA_count + 1;
        end if;
        
        if (IDCT_count = 7 and DATA_count = 3  ) THEN
         
           SRC_READY<='0';
           SRC_READY_SIG<='0';
           DATA_input_count <= 0;
           TMPBUF_FREE<='0';
           
        end if;
        
        if (DATA_count = 3 ) THEN
          PASS_R<='0';
          i0_r <= INBUF(IDCT_count*8);
          i1_r <= INBUF(1+IDCT_count*8);
          i2_r <= INBUF(2+IDCT_count*8);
          i3_r <= INBUF(3+IDCT_count*8);
          i4_r <= INBUF(4+IDCT_count*8);
          i5_r <= INBUF(5+IDCT_count*8);
          i6_r <= DATAIN (23 DOWNTO 16) & DATAIN (31 DOWNTO 24);
          i7_r <= DATAIN (7 DOWNTO 0) &DATAIN (15 DOWNTO 8);

        end if;
        
        end if;
        
        -- Has enough Data for an IDCT
        if (DATA_count > 3  ) THEN
          PASS_R<='0';
          IDCT_count <= IDCT_count + 1;
          if (SRC_VALID = '1') THEN
          DATA_count <= DATA_count - 3;
          end if;
          
          if (SRC_VALID = '0') THEN
          DATA_count <= DATA_count - 4;
          end if;
          
          IDCT_flag <= IDCT_flag + 1;
        end if;
        
        
        
            if (not(IDCT_flag=0)) THEN
        if(IDCT_timeout=0) THEN
          IDCT_flag <= IDCT_flag - 1;
          IDCT_timeout <=2;
          end if;

      end if;
      if(not(IDCT_timeout=0)) THEN
          IDCT_timeout<=IDCT_timeout-1;
      end if;

        
        if(IDCT_timeout=1 and sweep_horizontal_count=0) THEN
          TMPBUF(8*TMPBUF_count+0)<=o0_r;
          TMPBUF(8*TMPBUF_count+1)<=o1_r;
          TMPBUF(8*TMPBUF_count+2)<=o2_r;
          TMPBUF(8*TMPBUF_count+3)<=o3_r;
          TMPBUF(8*TMPBUF_count+4)<=o4_r;
          TMPBUF(8*TMPBUF_count+5)<=o5_r;
          TMPBUF(8*TMPBUF_count+6)<=o6_r;
          TMPBUF(8*TMPBUF_count+7)<=o7_r;
          TMPBUF_count<=TMPBUF_count+1;
          outbuf0 <= o0_r & o1_r & o2_r & o3_r;
          outbuf1 <= o4_r & o5_r & o6_r & o7_r;
      end if;
      
      if (TMPBUF_count = 8 ) THEN
           IDCT_count<=0;
           DATA_count<=0;
           
           end if;
           
      if (TMPBUF_count = 8 and OUTBUF_FREE='1' and sweep_horizontal_count < 8 ) THEN
          PASS_R<='1';
          i0_r <= TMPBUF(sweep_horizontal_count);
          i1_r <= TMPBUF(8+sweep_horizontal_count);
          i2_r <= TMPBUF(16+sweep_horizontal_count);
          i3_r <= TMPBUF(24+sweep_horizontal_count);
          i4_r <= TMPBUF(32+sweep_horizontal_count);
          i5_r <= TMPBUF(40+sweep_horizontal_count);
          i6_r <= TMPBUF(48+sweep_horizontal_count);
          i7_r <= TMPBUF(56+sweep_horizontal_count);
          sweep_horizontal_count <= sweep_horizontal_count+1;
          finbuf0 <= i0_r & i1_r & i2_r & i3_r;
          finbuf1 <= i4_r & i5_r & i6_r & i7_r;
          if (sweep_horizontal_count =3)THEN
          fetch_outbuf<=8;
          end if;
          
       end if;   
       
      if (sweep_horizontal_count =8) THEN
        sweep_horizontal_count <= 0;
        TMPBUF_count <= 0;
        OUTBUF_FREE<='0';
        TMPBUF_FREE<='1';
      end if;
      
      if (not(fetch_outbuf=0))THEN
          OUTBUF(8-fetch_outbuf)<=o0_r;
          OUTBUF(8-fetch_outbuf+8)<=o1_r;
          OUTBUF(8-fetch_outbuf+16)<=o2_r;
          OUTBUF(8-fetch_outbuf+24)<=o3_r;
          OUTBUF(8-fetch_outbuf+32)<=o4_r;
          OUTBUF(8-fetch_outbuf+40)<=o5_r;
          OUTBUF(8-fetch_outbuf+48)<=o6_r;
          OUTBUF(8-fetch_outbuf+56)<=o7_r;
          fetch_outbuf<=fetch_outbuf-1;
          
          fin4realbuf0 <= o0_r & o1_r & o2_r & o3_r;
          fin4realbuf1 <= o4_r & o5_r & o6_r & o7_r;
          dma_output_count<=dma_output_count+2;
      end if;
      
      if (fetch_outbuf=0 and dma_output_count>0) THEN
          DST_VALID <= '1';
          DST_VALID_SIG<= '1';
          
        



        if ((not(LAST_DST_READY_STATE='0' and DST_READY='1')) or dma_output_count=1 or (dma_output_count=16 and LAST_DST_READY_STATE='1') or testtest='0')THEN
          DATAOUT<= OUTBUF((16-dma_output_count)*4+0)(7 DOWNTO 0)&
                    OUTBUF((16-dma_output_count)*4+1)(7 DOWNTO 0)&
                    OUTBUF((16-dma_output_count)*4+2)(7 DOWNTO 0)&
                    OUTBUF((16-dma_output_count)*4+3)(7 DOWNTO 0);
                    
          end if;
          
          
		if(LAST_DST_READY_STATE='1')THEN
	  		testtest<='1';
      
      	end if;
          
          
          if (LAST_DST_READY_STATE='0' and DST_READY='1' and not(dma_output_count=1 or dma_output_count=16)) THEN
          DATAOUT<= OUTBUF((16-dma_output_count)*4+4)(7 DOWNTO 0)&
                    OUTBUF((16-dma_output_count)*4+5)(7 DOWNTO 0)&
                    OUTBUF((16-dma_output_count)*4+6)(7 DOWNTO 0)&
                    OUTBUF((16-dma_output_count)*4+7)(7 DOWNTO 0);
          
          end if;
          if(testtest='1' and not (dma_output_count=1 ))
          THEN
          DATAOUT<= OUTBUF((16-dma_output_count)*4+4)(7 DOWNTO 0)&
                    OUTBUF((16-dma_output_count)*4+5)(7 DOWNTO 0)&
                    OUTBUF((16-dma_output_count)*4+6)(7 DOWNTO 0)&
                    OUTBUF((16-dma_output_count)*4+7)(7 DOWNTO 0);
          
          end if;

           if(TMPBUF_FREE='1' and not(IDCT_count = 7 and DATA_count = 3)) THEN
            SRC_READY<='1';
            SRC_READY_SIG<='1';
           end if;
	 
           
      end if;
      
      
      
      if(DST_READY='0' or (DST_VALID_SIG='0' and LAST_DST_VALID_SIG='1'))THEN
      testtest<='0';
      end if;
      

      if(DST_VALID_SIG= '1' and DST_READY ='1' and dma_output_count>1 )THEN
        dma_output_count <= dma_output_count - 1;
      end if;
      
      
      
          LAST_DST_READY_STATE<=DST_READY;
		LAST_DST_VALID_SIG<=DST_VALID_SIG;
      
      if(DST_VALID_SIG= '1' and DST_READY ='1' and dma_output_count=1 )THEN
        DST_VALID_SIG <= '0';
        DST_VALID<='0';
        OUTBUF_FREE<='1';
        dma_output_count<=0;
        testtest<='0';
      end if;
                    
      end process;
      
        end architecture a_idct_2D;