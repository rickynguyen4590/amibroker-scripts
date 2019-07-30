global ColNumber;
ON = 1;
OFF = 0;

function  ElapsedTime() { 
	TimeElapse = Now(4);
	MinuteElapsed = floor(TimeElapse /10000)*60 + floor((TimeElapse - floor(TimeElapse /10000) *10000)/100) - 540; 

	if (MinuteElapsed >0 AND MinuteElapsed <=150) {
		RelativeTimeElapsed = MinuteElapsed;
	}
	else if (MinuteElapsed >150 AND MinuteElapsed <=240) {
		RelativeTimeElapsed = 150;
	}
	else if (MinuteElapsed >240 AND MinuteElapsed <=330) {
		RelativeTimeElapsed = MinuteElapsed - 90;
	}
	else if (MinuteElapsed >330 AND MinuteElapsed <=345) {
		RelativeTimeElapsed = 240;
	}
	else {
		RelativeTimeElapsed = 255;
	}

	return RelativeTimeElapsed;
}

function  TransactionSpeed() {
	RelativeTimeElapsed = ElapsedTime();

	bi = BarIndex();
	lvbi = LastValue( bi );

	Speed = IIf( bi > lvbi-1, V/RelativeTimeElapsed , V/255  );
	
	return Speed;
}

function  ExpectedVolume() { 
	
	RelativeTimeElapsed = ElapsedTime() /255;

	bi = BarIndex();
	lvbi = LastValue( bi );

	ExpectedVol= IIf( bi > lvbi-1, V, V*RelativeTimeElapsed  );
	return ExpectedVol / RelativeTimeElapsed ;
}

function MarketCapitalization(price, unit) {
	return price*OI/unit;
} 


function Pressure(PH, PL, PC, PV) {
	sp = PH-PC;
	bp = PC-PL;
	CC =  Name() + "_CC";
	vb = Prec(Foreign(CC,"C",0),0);
	vs = Prec(Foreign(CC,"O",0),0);
	vo = vb/(vs+0.001);
	vofactor = 1 - 1/(1+vo);
	 
	PP = Param("Pressure Period ", 40, 2, 200 );
	SP = Param("Smooth Period ", 14, 2, 200 );
	
	vofavg = TEMA(vofactor, PP);

	bpavg = TEMA(bp,PP);
	spavg = TEMA(sp,PP);
	nbp = bp/bpavg;
	nsp = sp/spavg;
	
	Varg = TEMA(PV,PP );
	nv = V/Varg;
	nbfraw = nbp * (nv + IIf(IsNull(vb) AND IsNull(vs), 0,  vofavg ) );
	nsfraw = nsp * (nv + IIf(IsNull(vb) AND IsNull(vs), 0,  1-vofavg ) );
	global nbf; 
	nbf = EMA(nbfraw ,SP);
	global nsf;
	nsf = EMA(nsfraw ,SP);
	diff = nbf-nsf;
	return diff;
}

procedure kStaticVarSet( SName, SValue )         
    {
    ChartID = GetChartID();
    InIndicator = Status("Action") == 1;
    if( InIndicator ) StaticVarSet(Sname+ChartID, Svalue); 
    }

function kStaticVarGet( SName )                     
    { 
    ChartID     = GetChartID();
    Var = StaticVarGet(Sname+ChartID);
    return Var;
    }

procedure kStaticVarSetText( SName, SValue )     
    { 
    ChartID     = GetChartID();
    InIndicator = Status("Action") == 1;
    if( InIndicator ) StaticVarSetText(Sname+ChartID, Svalue); 
	 
    }

function kStaticVarGetText( SName )                 
    { 
    ChartID = GetChartID();
    return StaticVarGetText(Sname+ChartID); 
    }

function Column_Begin( ColName ) 
    {
    global FontRatio, ColName, ColNumber, CellHeight, CellWidth, PanelXoffset, PanelYoffset;
    ColNumber = VarGet("ColNumber");
    if( IsEmpty( ColNumber ) ) 
        {
        VarSet("ColNumber",1);
        StaticVarSet("ClickCoordinates",0);
        }
    else VarSet("ColNumber", ++ColNumber);
    ColName = ColName+GetChartID();
    GfxSetOverlayMode( 0 );
    GfxSelectFont( "Tahoma", CellHeight/FontRatio, 800 ); 
    GfxSelectPen( colorBlack ); 
    GfxSetBkMode( 1 );
    kStaticVarSet("RowNumber"+ColName, 0);
    VarSetText("ColName",ColName);
	 
    return ColNumber;
    }

function Column_End( )
    {
    global CellHeight, CellWidth, PanelYoffset, PanelXoffset, ColNumber, RowNumber;
    ChartIDStr     = NumToStr(GetChartID(),1.0,False);
    ColName         = VarGetText("ColName");
    ULCellX         = PanelXoffset + (ColNumber-1) * CellWidth;
    LRCellX        = ULCellX + CellWidth;
    for( Row = 1; Row <= RowNumber; Row++ ) 
        {
        ULCellY         = (Row-1) * CellHeight + PanelYoffset;
        LRCellY        = ULCellY + CellHeight;
        TextCell     = kStaticVarGetText("TextCell"+ColName+Row);
        if (TextCell =="")
			TextCell = "TEST MODUS OFF";
		 TextColor     = Nz(kStaticVarGet("TextColor"+ColName+Row), colorBlack);
        BackColor     = Nz(kStaticVarGet("BackColor"+ColName+Row), colorRed);
		 
        GfxSelectSolidBrush( BackColor);
        GfxRectangle( ULCellX, ULCellY, LRCellX, LRCellY ); 
        GfxSetBkColor( BackColor);
        GfxSetTextColor( TextColor );
        GfxDrawText( TextCell, ULCellX, ULCellY, LRCellX, LRCellY, 32 | 1 | 4);
        }
    }

function TextCell( TextCell, backColor, TextColor)
    {
    global ColNumber, RowNumber;;
    ColName = VarGetText("ColName");
    RowNumber = Nz(kStaticVarGet("RowNumber"+ColName))+1;
    kStaticVarSet("RowNumber"+ColName, RowNumber);
    kStaticVarSetText("TextCell"+ColName+RowNumber, TextCell);
    kStaticVarSet("TextColor"+ColName+RowNumber, TextColor);
    kStaticVarSet("BackColor"+ColName+RowNumber, backColor);
    }

function NewColumn()
    {
    VarSet("ColNumber", 0);
    }

function CheckMouseClick( ColNumber, RowNumber )
    {
    global PanelYoffset, PanelXoffset, CellHeight, CellWidth;
    LButtonDown = GetCursorMouseButtons() == 9;
    Click = 0;
    if( LButtonDown )
        {
        ULCellX         = PanelXoffset + (ColNumber-1) * CellWidth;
        LRCellX        = ULCellX + CellWidth;
        ULCellY         = (RowNumber -1) * CellHeight + PanelYoffset;
        LRCellY        = ULCellY + CellHeight;
        MouseCoord = Nz(StaticVarGet("ClickCoordinates"));
        if( MouseCoord == 0 AND LButtonDown )
            {
            MousePx = GetCursorXPosition( 1 );
            MousePy = GetCursorYPosition( 1 );
            if( MousePx > ULCellX AND MousePx < LRCellX AND MousePy > ULCellY AND MousePy < LRCellY )
                {
                StaticVarSet("ClickCoordinates",ColNumber*100+RowNumber);
                Click = 1;
                }
            }
        }
    return Click;
    }

function TriggerCell( Label1, Label2, backColor1, backColor2, TextColor)
{
	global ColNumber, RowNumber;;
	ColName = VarGetText("ColName");
	RowNumber = Nz(kStaticVarGet("RowNumber"+ColName))+1;
	kStaticVarSet("RowNumber"+ColName, RowNumber);
	Trigger = CheckMouseClick( ColNumber, RowNumber );
	Label = Label1;
    if( Trigger ) {
	if (kStaticVarGet("ONCLICK") == 1) {
		kStaticVarSet("ONCLICK", 0);
		BackColor = backColor1; 
		Label = Label1;
		StaticVarSet("TestModus", 0);
	}
	else {
		kStaticVarSet("ONCLICK", 1);
		BackColor = backColor2; 
		Label = Label2;
		StaticVarSet("TestModus", 1);
	}
   
    kStaticVarSetText("TextCell"+ColName+RowNumber, Label);
    kStaticVarSet("TextColor"+ColName+RowNumber, TextColor); 
    kStaticVarSet("BackColor"+ColName+RowNumber, BackColor);
    }
    return Trigger;
}  

function T3(price,periods) //AMA-based
{
s=0.618; //Param("Hot ?",0.618,0,100,0.001,0);

e1=FA_AMAb(price,periods);
e2=FA_AMAb(e1,periods);
e3=FA_AMAb(e2,periods);
e4=FA_AMAb(e3,periods);
e5=FA_AMAb(e4,periods);
e6=FA_AMAb(e5,periods);
x= 3*s;
y = 3*s*s;
z = s*s*s;
c2 = y + 3*z;
c3 =-2*y-x-3*z;
c4=1+x+y+z;
ti3=-z*e6+c2*e5+c3*e4+c4*e3;
return ti3;
}

function DarvasHigh( PriceHigh, Periods )
{
	HHVtemp = HHV( PriceHigh, Periods );
	result = HHVTemp;
	for( i = Periods + 4; i < BarCount; i++ )
	{
   		result[ i ] = IIf( H[ i - 3 ] >= HHVTemp[ i - 4 ] AND
                     PriceHigh[ i - 3 ] > PriceHigh[ i - 2 ] AND
                     PriceHigh[ i - 3 ] > PriceHigh[ i - 1 ] AND
                     PriceHigh[ i - 3 ] > PriceHigh[ i ],
                     PriceHigh[ i - 3 ],
                     result[ i - 1 ] );
	}
	return result;
}

function NewDarvasHigh(PriceHigh, Periods )
{
  dh = DarvasHigh(PriceHigh, Periods );
  //return dh AND Nz( dh ) != Ref( Nz( dh ), -1 );
 	return ValueWhen( Nz( dh ) != Ref( Nz( dh ), -1 ), Ref( PriceHigh, -3 ) );
}

function DarvasLow( PriceHigh, PriceLow, Periods )
{
  dh = DarvasHigh( PriceHigh, Periods );
  ndl = Ref( PriceLow , -3 ) < Ref( PriceLow , -2 ) AND
        Ref( PriceLow , -3 ) < Ref( PriceLow, -1 ) AND
        Ref( PriceLow , -3 ) < PriceLow AND
        Ref( PriceHigh, -2 ) < dh AND
        Ref( PriceHigh, -1 ) < dh AND
         PriceHigh < dh;
  return Nz( ndl ) AND Ref( Nz( ndl ), -1 ) < 1;
}

function NewDarvasLow(PriceHigh, PriceLow, Periods )
{
  return ValueWhen( DarvasLow( PriceHigh, PriceLow, Periods ), Ref( PriceLow, -3 ) );
}

function DarvasBoxEnd(PriceHigh, PriceLow, Periods )
{
	end = BarsSince( NewDarvasHigh( PriceHigh, Periods ) ) <
       	BarsSince( Ref( DarvasLow( PriceHigh, PriceLow, Periods ), -1 ) );
	return Nz( end ) AND DarvasLow( PriceHigh, PriceLow, Periods );
}

function DarvasBoxHigh(  PriceHigh, PriceLow, Periods )
{
	dbe = DarvasBoxEnd( PriceHigh, PriceLow, Periods );
	dbhi = ValueWhen( Nz( dbe ) AND NOT IsNull( Ref( dbe, -1 ) ),
       	DarvasHigh( PriceHigh,  Periods ) );
	return IIf( Nz( dbhi ) == 0, H + 1e-6, dbhi );
}

function DarvasBoxLow( PriceHigh, PriceLow, Periods )
{
	dbe = DarvasBoxEnd( PriceHigh, PriceLow, Periods );
	dblo = ValueWhen( Nz( dbe ) AND NOT IsNull( Ref( dbe, -1 ) ),
        NewDarvasLow( PriceHigh, PriceLow, Periods ) );
	return IIf( Nz( dblo ) == 0, L - 1e-6, dblo );
}

function DarvasPossSell( PriceHigh, PriceLow, Periods )
{
	dsl = Low < DarvasBoxLow( PriceHigh, PriceLow, Periods );
	return Nz( dsl ) AND Ref( Nz( dsl ), -1 ) < Nz( dsl );
}

function GetSecondNum()
{
    Time 		= Now( 4 );
    Seconds 	= int( Time % 100 );
    Minutes 	= int( Time / 100 % 100 );
    Hours 	= int( Time / 10000 % 100 );
    SecondNum = int( Hours * 60 * 60 + Minutes * 60 + Seconds );
    return SecondNum;
}

