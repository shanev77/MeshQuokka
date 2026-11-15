"""Render the complete manual with contents, bookmarks and repeating table headers."""
from pathlib import Path
import re, textwrap
from datetime import datetime
from xml.sax.saxutils import escape
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import Image as ReportImage, SimpleDocTemplate, Paragraph, Spacer, PageBreak, Table, TableStyle, Preformatted
from reportlab.platypus.tableofcontents import TableOfContents

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'release/MeshQuokka-User-Manual.pdf'
VERSION=re.search(r'0\.1\.0_\d{6}[a-z]',(ROOT/'platformio.local.ini').read_text()).group()
RELEASE_DATE=datetime.strptime(VERSION.split('_')[1][:6],'%y%m%d').strftime('%d %B %Y')
INK=colors.HexColor('#12304a'); TEAL=colors.HexColor('#087f8c'); WIDTH=A4[0]-84
styles=getSampleStyleSheet()
styles.add(ParagraphStyle('BodyM',fontName='Helvetica',fontSize=9.5,leading=14,spaceAfter=8,textColor=INK))
styles.add(ParagraphStyle('ListM',parent=styles['BodyM'],leading=13,spaceAfter=5))
styles.add(ParagraphStyle('H1M',fontName='Helvetica-Bold',fontSize=22,leading=27,spaceAfter=18,textColor=INK,keepWithNext=True))
styles.add(ParagraphStyle('H2M',fontName='Helvetica-Bold',fontSize=12,leading=16,spaceBefore=11,spaceAfter=7,textColor=TEAL,keepWithNext=True))
styles.add(ParagraphStyle('CellM',parent=styles['BodyM'],fontSize=8.7,leading=12,spaceAfter=0))
styles.add(ParagraphStyle('CodeM',fontName='Courier',fontSize=7.3,leading=10.5,spaceAfter=12,backColor=colors.HexColor('#eef3f6'),borderPadding=7))
styles.add(ParagraphStyle('CoverM',fontName='Helvetica-Bold',fontSize=38,leading=44,textColor=INK,spaceAfter=20))
styles.add(ParagraphStyle('TocM',fontName='Helvetica',fontSize=9.5,leading=14,textColor=INK,leftIndent=0,firstLineIndent=0,spaceBefore=0))

def inline(value):
    value=escape(value)
    value=re.sub(r'\*\*(.+?)\*\*',r'<b>\1</b>',value)
    value=re.sub(r'`(.+?)`',r'<font name="Courier">\1</font>',value)
    return re.sub(r'(https://[^\s<]+)',r'<link href="\1" color="#087f8c">\1</link>',value)

def markdown(path):
    lines=path.read_text(encoding='utf-8').splitlines();out=[];i=0;chapter=0
    while i<len(lines):
        line=lines[i].strip()
        if not line:i+=1;continue
        if line.startswith('# '):
            if chapter:out.append(PageBreak())
            chapter+=1
            p=Paragraph(inline(line[2:]),styles['H1M']);p.chapter_title=line[2:];p.chapter_key=f'chapter-{chapter}'
            out.append(p);i+=1;continue
        if line.startswith('## '):
            out.append(Paragraph(inline(line[3:]),styles['H2M']));i+=1;continue
        if line.startswith('```'):
            i+=1;code=[]
            while i<len(lines) and not lines[i].startswith('```'):
                parts=textwrap.wrap(lines[i],width=108,break_long_words=False,break_on_hyphens=False,subsequent_indent='    ') or ['']
                code.extend([part+' '+chr(96) if j<len(parts)-1 else part for j,part in enumerate(parts)])
                i+=1
            out.append(Preformatted('\n'.join(code),styles['CodeM']));i+=1;continue
        if line.startswith('|'):
            rows=[]
            while i<len(lines) and lines[i].strip().startswith('|'):
                cells=[c.strip() for c in lines[i].strip().strip('|').split('|')]
                if not all(re.fullmatch(r'[: -]+',c) for c in cells):rows.append([Paragraph(inline(c),styles['CellM']) for c in cells])
                i+=1
            n=len(rows[0]);widths=[WIDTH*.31,WIDTH*.69] if n==2 else [WIDTH/n]*n
            table=Table(rows,colWidths=widths,repeatRows=1,hAlign='LEFT')
            table.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),colors.HexColor('#dcecef')),
                ('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),
                ('TOPPADDING',(0,0),(-1,-1),4.5),('BOTTOMPADDING',(0,0),(-1,-1),4.5),
                ('LINEBELOW',(0,0),(-1,0),.8,TEAL),('ROWBACKGROUNDS',(0,1),(-1,-1),[colors.white,colors.HexColor('#f2f6f8')])]))
            out.extend([table,Spacer(1,10)]);continue
        parts=[line];i+=1
        while i<len(lines) and lines[i].strip() and not re.match(r'^(#|\||```|\* |\d+\. |- )',lines[i].strip()):
            parts.append(lines[i].strip());i+=1
        value=' '.join(parts);is_list=bool(re.match(r'^(\* |- |\d+\. )',value))
        if value.startswith('* '):value='- '+value[2:]
        out.append(Paragraph(inline(value),styles['ListM' if is_list else 'BodyM']))
    return out

class ManualDoc(SimpleDocTemplate):
    def afterFlowable(self,flowable):
        if hasattr(flowable,'chapter_key'):
            key=flowable.chapter_key;title=flowable.chapter_title
            self.canv.bookmarkPage(key);self.canv.addOutlineEntry(title,key,0,False)
            self.notify('TOCEntry',(0,title,self.page,key))

def chrome(canvas,doc):
    canvas.saveState();w,h=A4
    canvas.setStrokeColor(TEAL);canvas.setLineWidth(.8);canvas.line(42,h-33,w-42,h-33)
    canvas.setFont('Helvetica',7.5);canvas.setFillColor(colors.HexColor('#536676'))
    canvas.drawString(42,h-25,'MESHQUOKKA  /  T-DECK PLUS 915 MHz')
    canvas.drawString(42,24,'User and technical reference | '+VERSION+' | '+RELEASE_DATE)
    canvas.drawRightString(w-42,24,str(doc.page));canvas.restoreState()

story=[Spacer(1,12),ReportImage(str(ROOT/'assets/MeshQuokka-Logo.png'),width=82,height=82),Spacer(1,12),Paragraph('MeshQuokka',styles['CoverM']),
    Paragraph('Data Packet Radio on the<br/>MESHCORE network',styles['H1M']),Spacer(1,20),
    Paragraph('LilyGO T-Deck Plus | 915 MHz<br/>By S. van Ingen<br/>Development '+VERSION+'<br/>'+RELEASE_DATE,styles['BodyM']),Spacer(1,25),
    Paragraph('Use it. Understand it. Recover it. Build it.',styles['H2M']),
    Paragraph('All Home apps and Settings options, messaging, Bluetooth, GPS diagnostics, Wi-Fi, maps, terminal commands, flashing, hardware, software architecture and GitHub source distribution.',styles['BodyM']),Spacer(1,25),
    Paragraph('<b>Validation:</b> Back and Android connection are user-confirmed. GPS positioning is confirmed outdoors and in live diagnostics. Street maps add on-demand Wi-Fi downloads and SD caching. Full peer and sustained-load qualification remains open.',styles['BodyM']),
    Paragraph('<b>Recovery:</b> preserve a verified full-flash backup before replacing unknown firmware. Private device data is excluded from the source package.',styles['BodyM']),PageBreak(),Paragraph('Contents',styles['H1M'])]
toc=TableOfContents();toc.levelStyles=[styles['TocM']];story.extend([toc,PageBreak()]);story.extend(markdown(ROOT/'docs/USER_MANUAL.md'))
OUT.parent.mkdir(exist_ok=True)
doc=ManualDoc(str(OUT),pagesize=A4,rightMargin=42,leftMargin=42,topMargin=49,bottomMargin=44,
    title='MeshQuokka '+VERSION+' - Complete User and Technical Manual',author='S. van Ingen')
doc.multiBuild(story,onFirstPage=chrome,onLaterPages=chrome)
print(OUT)
