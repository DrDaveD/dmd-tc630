#include	"jerq.h"
#include	"menu.h"
#define		disp Jfscreen

#ifdef	SUNTOOLS
#undef F_STORE
#define	F_STORE		((displaypw->pw_pixrect->pr_depth==1) ? \
			(PIX_DONTCLIP|PIX_SRC) : PIX_SRC)
#endif	/* SUNTOOLS */

#define scale(x, inmin, inmax, outmin, outmax)\
	(outmin + muldiv(x-inmin,outmax-outmin,inmax-inmin))

#define bound(x, low, high) min(high, max( low, x ))

Font *font = &defont;
#define SPACING		(1+fontheight(font))
#define CHARWIDTH	ave_fontwid(font)
#define DISPLAY		16
#define DELTA		6
#define BARWIDTH	18
#define	MAXDEPTH	16	/* don't use too much stack */
#define ARROWIDTH	20
#define CHECKWIDTH	20

static unsigned short tarrow_bits[] =
{
	0x0000, 0x0000, 0x0080, 0x00C0, 0x00C0, 0x00E0, 0x00F8, 0xFFFE,
	0xFFFE, 0x00F8, 0x00E0, 0x00C0, 0x00C0, 0x0080, 0x0000, 0x0000,
};
static Texture tarrow;
static int firstime = 1;

static short checkmark[] = {            /* check mark */
         0x0000, 0x0003, 0x0007, 0x000E,
         0x001C, 0x0038, 0x0070, 0x00E0,
         0x61E0, 0x73C0, 0x3F80, 0x3F00,
         0x1E00, 0x1E00, 0x0C00, 0x0000,
};
static Texture tcheckmark;

	static NMitem *
tablegen(i, table)
	NMitem *table;
{
	return &table[i];
}

static char fill[64];

	NMitem *
nmhit(m, but, depth)
	register NMenu *m;
{
	register int width, mwidth, i, j, top, newtop, hit, newhit;
	register int items, lines, length, w, x;
	Point p, q, savep, baro, barc;
	Rectangle sr, tr, mr;	/* scroll, text, menu */
	Rectangle rside, rhit;
	register Bitmap *b;
	register char *from, *to;
	Bitmap *bhelp = 0, *arrow, *B_checkmark;
	NMitem *(*generator)(), *mi, *table, *ret = 0;
	int dohfn;
	int selmnu;

#define sro sr.origin
#define src sr.corner
#define tro tr.origin
#define trc tr.corner
#define mro mr.origin
#define mrc mr.corner

	if (firstime) {
		firstime = 0;
		tarrow = ToTexture((short *)tarrow_bits);
		tcheckmark = ToTexture(checkmark);
	}
	arrow = balloc(Rect(0, 0, 16, 16));
	texture(arrow, arrow->rect, &tarrow, F_STORE);
	B_checkmark = balloc(Rect(0, 0, 16, 16));
	texture(B_checkmark, B_checkmark->rect, &tcheckmark, F_STORE);

	generator = (table=m->item) ? tablegen : m->generator;
	selmnu = w = x = length = 0;
	for(items = 0; (mi=(*generator)(items, table))->text; ++items) {
		register int s = strlen (mi->text);
		length = max(length, s);
		if (mi->next)
			w = max (w, s);
		else
			x = max (x, s);
		if (mi->selected)
			selmnu = 1;
	}
	if(items == 0)
		return(ret);
	Jscreengrab();
	width = length*CHARWIDTH+10;
	w *= CHARWIDTH;
	x *= CHARWIDTH;
	if (x <= w)
		mwidth = w + ARROWIDTH;
	else if (x >= w + 2*ARROWIDTH)
		mwidth = x;
	else
		mwidth = w + ARROWIDTH + (x - w) / 2;
	mwidth += 10;
	if (selmnu)
		mwidth += CHECKWIDTH;
	sro.x = sro.y = src.x = tro.x = mro.x = mro.y = 0;
	if(items <= DISPLAY)
		lines = items;
	else{
		lines = DISPLAY;
		tro.x = src.x = BARWIDTH;
		sro.x = sro.y = 1;
	}
#define ASCEND 2
	tro.y = ASCEND;
	tro.x += selmnu * CHECKWIDTH;
	mrc = trc = add(tro, Pt(mwidth, min(items, lines)*SPACING));
	src.y = mrc.y-1;
	newtop = bound(m->prevtop, 0, items-lines);
	p = add(mouse.xy, Joffset);
	p.y -= bound(m->prevhit, 0, lines-1)*SPACING+SPACING/2;
	p.x = bound(p.x-(src.x+mwidth/2), disp.rect.origin.x,
		disp.rect.corner.x-mrc.x);
	p.y = bound(p.y, disp.rect.origin.y, disp.rect.corner.y-mrc.y);
	sr = raddp(sr, p);
	tr = raddp(tr, p);
	mr = raddp(mr, p);
	rside = mr;
	rside.origin.x = rside.corner.x-20;
	b = balloc(mr);
	if(b)
		bitblt(&disp, mr, b, mro, F_STORE);
	rectf(&disp, mr, F_OR);
PaintMenu:
	rectf(&disp, inset(mr, 1), F_CLR);
	top = newtop;
	if(items > DISPLAY){
		baro.y = scale(top, 0, items, sro.y, src.y);
		baro.x = sr.origin.x;
		barc.y = scale(top+DISPLAY, 0, items, sro.y, src.y);
		barc.x = sr.corner.x;
		rectf(&disp, Rpt(baro,barc), F_XOR);
	}
	for(p=tro, i=top; i < min(top+lines, items); ++i){
		q = p;
		mi = generator(i, table);
		from = mi->text;
		for(to = &fill[0]; *from; ++from)
			if(*from & 0x80)
				for(j=length-(strlen(from+1)+(to-&fill[0])); j-->0;)
					*to++ = *from & 0x7F;
			else
				*to++ = *from;
		*to = '\0';
		q.x += (width-strwidth(font,fill))/2;
		string(&defont, fill, &disp, q, F_XOR);
		if(mi->next)
			bitblt(arrow, arrow->rect, &disp, Pt(trc.x-18, p.y-2), F_OR);
		if(mi->selected)
			bitblt(B_checkmark, B_checkmark->rect, &disp,
				Pt(tro.x-18, p.y-2), F_OR);
		p.y += SPACING;
	}
	savep = add(mouse.xy, Joffset);
	mi = 0;
	for(newhit = hit = -1; button(but); ){
		p = add(mouse.xy, Joffset);
		if(depth && ((p.x < mro.x) || button(5-but)))
		{
			ret = 0;
			break;
		}
		if(ptinrect(p, sr)){
			if(ptinrect(savep,tr)){
				p.y = (baro.y+barc.y)/2;
				cursset(sub(p,Joffset));
			}
			newtop = scale(p.y, sro.y, src.y, 0, items);
			newtop = bound(newtop-DISPLAY/2, 0, items-DISPLAY);
			if(newtop != top)
/* ->->-> */			goto PaintMenu;
		}else if(ptinrect(savep,sr)){
			register dx, dy;
			if(abs(dx = p.x-savep.x) < DELTA)
				dx = 0;
			if(abs(dy = p.y-savep.y) < DELTA)
				dy = 0;
			if(abs(dy) >= abs(dx))
				dx = 0;
			else
				dy = 0;
			p = add(savep, Pt(dx,dy));
			cursset(sub(p,Joffset));
		}
		savep = p;
		newhit = -1;
		if(ptinrect(p, tr)){
			newhit = bound((p.y-tro.y)/SPACING, 0, lines-1);
			if(newhit!=hit && hit>=0
			 && abs(tro.y+SPACING*newhit+SPACING/2-p.y) > SPACING/3)
				newhit = hit;
			rhit = tr;
			rhit.origin.y += newhit*SPACING-ASCEND/2;
			rhit.corner.y = rhit.origin.y + SPACING;
		}
		if(newhit == -1)
			ret = 0, dohfn = 0;
		else
			ret = mi = (*generator)(top+newhit, table), dohfn = 1;
		if(newhit == hit)
		{
			if((newhit != -1) && (bhelp == 0) && button1() && mi->help)
				helpon(mi->help, rhit, &bhelp);
			else if(bhelp && !button1())
				helpoff(&bhelp);
		}
		else
		{
			Rectangle temp;

			temp = tr;
			if (selmnu)
				temp.origin = sub(temp.origin, 
						  Pt(CHECKWIDTH,0));
			flip(temp, hit);
			helpoff(&bhelp);
			flip(temp, newhit);
			hit = newhit;
			if((newhit != -1) && button1() && mi->help)
				helpon(mi->help, rhit, &bhelp);
		}
		if((newhit != -1) && ptinrect(p, rside))
		{
			if(mi->dfn) (*mi->dfn)(mi);
			if(mi->next && (depth <= MAXDEPTH))
				ret = nmhit(mi->next, but, depth+1), dohfn = 0;
			if(mi->bfn) (*mi->bfn)(mi);
		}
		if(newhit==0 && top>0){
			newtop = top-1;
			p.y += SPACING;
			cursset(sub(p,Joffset));
/* ->->-> */		goto PaintMenu;
		}
		if(newhit==DISPLAY-1 && top<items-lines){
			newtop = top+1;
			p.y -= SPACING;
			cursset(sub(p,Joffset));
/* ->->-> */		goto PaintMenu;
		}
		if (button(but))
			nap(2);
	}
	if(bhelp)
		helpoff(&bhelp);
	if(b){
		bitblt(b, b->rect, &disp, b->rect.origin, F_STORE);
		bfree(b);
	}
	if(hit>=0){
		m->prevhit = hit;
		m->prevtop = top;
		if(ret && ret->hfn && dohfn) (*ret->hfn)(mi);
	}
	Jscreenrelease();
	return(ret);
}

static
flip(r,n)
	Rectangle r;
{
	if(n<0)
		return;
	++r.origin.x;
	r.corner.y = (r.origin.y += SPACING*n-1) + SPACING;
	--r.corner.x;
	rectf(&disp, r, F_XOR);
}

static
helpon(msg, r, bhelp)
	char *msg;
	Rectangle r;
	Bitmap **bhelp;
{
	register w;

	w = strwidth(&defont, msg)+10;
	if(r.corner.x+w < disp.rect.corner.x){
		r.origin.x = r.corner.x;
		r.corner.x += w;
	}
	else{
		r.corner.x = r.origin.x;
		r.origin.x -= w;
	}
	if(*bhelp = balloc(r = inset(r, -1))){
		bitblt(&disp, r, *bhelp, r.origin, F_STORE);
		rectf(&disp, r, F_OR);
		rectf(&disp, inset(r, 1), F_XOR);
		string(&defont,msg,&disp,add(r.origin,
			Pt(5, 1)),F_XOR);
	}
}

static
helpoff (bhelp)
Bitmap **bhelp;
{
	Bitmap *bh;

	if(bh = *bhelp){
		bitblt(bh, bh->rect, &disp, bh->rect.origin, F_STORE);
		bfree(bh);
		*bhelp = 0;
	}
}
