/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkVoronoiDiagram2DGenerator.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef _itkVoronoiDiagram2DGenerator_txx
#define _itkVoronoiDiagram2DGenerator_txx


#include <algorithm>
#include "vnl/vnl_sample.h"

namespace itk{

const double NUMERIC_TOLERENCE = 1.0e-10;
const double DIFF_TOLERENCE = 0.001;

template <typename TCoordRepType>
VoronoiDiagram2DGenerator<TCoordRepType>::
VoronoiDiagram2DGenerator()
{
  m_NumberOfSeeds = 0;
  f_pxmin = 0;
  f_pymin = 0;
  m_OutputVD=this->GetOutput();
}

template <typename TCoordRepType>
VoronoiDiagram2DGenerator<TCoordRepType>::
~VoronoiDiagram2DGenerator()
{
}

template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Number Of Seeds: " 
     << m_NumberOfSeeds << std::endl;
}

/* set random seed points, specify the number of seeds as "num" */
template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
SetRandomSeeds(int num)
{
  PointType curr;
  m_Seeds.clear();
  double ymax = (double)(m_VorBoundary[1]);
  double xmax = (double)(m_VorBoundary[0]);
  for(int i = 0; i < num; ++i){
    curr[0] = (CoordRepType)(vnl_sample_uniform(0,xmax));
    curr[1] = (CoordRepType)(vnl_sample_uniform(0,ymax));
    m_Seeds.push_back(curr);
  }
  m_NumberOfSeeds = num;
}

/* set the seed points, specify the number of seeds as "num" */
template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
SetSeeds(int num,  SeedsIterator begin)
{
  m_Seeds.clear();
  SeedsIterator ii(begin);
  for(int i = 0; i < num; ++i){
    m_Seeds.push_back(*ii++);
  }
  m_NumberOfSeeds = num;
}

/* set the rectangle that enclosing the Voronoi Diagram. */
template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
SetBoundary(PointType vorsize)
{
  m_VorBoundary[0] = vorsize[0];
  m_VorBoundary[1] = vorsize[1];
  m_OutputVD->SetBoundary(vorsize); 
}

template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
SetOrigin(PointType vorsize)
{
  f_pxmin = vorsize[0];
  f_pymin = vorsize[1];
  m_OutputVD->SetOrigin(vorsize); 
}

/* compare two Point coordinates by the y coordinates */ 
template <typename TCoordRepType>
bool
VoronoiDiagram2DGenerator<TCoordRepType>::
comp(PointType arg1,PointType arg2){
  if(arg1[1]<arg2[1]) return 1;
  else if(arg1[1]>arg2[1]) return 0;
  else if(arg1[0]<arg2[0]) return 1;
  else if(arg1[0]>arg2[0]) return 0;
  else return 1;
}


/* sort the seeds by their y coordinates */
template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
SortSeeds(void)
{
  std::sort(m_Seeds.begin(),m_Seeds.end(),comp);
}


/* add seeds, specify the number of seeds to be added as "num" */
template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
AddSeeds(int num,  SeedsIterator begin)
{
  SeedsIterator ii(begin);
  for(int i = 0; i < num; ++i){
    m_Seeds.push_back(*ii++);
  }
  m_NumberOfSeeds += num;
}

/* add one seed */
template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
AddOneSeed(PointType inputSeed)
{
  m_Seeds.push_back(inputSeed);
  m_NumberOfSeeds++;
}

template <typename TCoordRepType>
VoronoiDiagram2DGenerator<TCoordRepType>::PointType 
VoronoiDiagram2DGenerator<TCoordRepType>::
GetSeed(int SeedID){
  PointType answer;
  answer[0]=m_Seeds[SeedID][0];
  answer[1]=m_Seeds[SeedID][1];
  return answer;
}

template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
GenerateData(void)
{
  SortSeeds();
  m_OutputVD->SetSeeds(m_NumberOfSeeds, m_Seeds.begin());
  GenerateVDFortune();
  ConstructDiagram();
}

template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
UpdateDiagram(void)
{
  GenerateData();
}


/*************************************************************/
/*************************************************************/
/* methods that convert the result from Fortune Algorithm to 
 * itkMesh
 */
template <typename TCoordRepType>
bool
VoronoiDiagram2DGenerator<TCoordRepType>::
differentPoint(PointType p1,PointType p2){
  double diffx = p1[0]-p2[0];
  double diffy = p1[1]-p2[1];
  return ( (diffx<-DIFF_TOLERENCE) || (diffx>DIFF_TOLERENCE) 
         || (diffy<-DIFF_TOLERENCE) || (diffy>DIFF_TOLERENCE) );   
  
/*  if( ((int)(p1[0]) != (int)(p2[0])) || ((int)(p1[1]) != (int)(p2[1])) )
    return 1;
  return 0;
  */
}

template <typename TCoordRepType>
bool
VoronoiDiagram2DGenerator<TCoordRepType>::
almostsame(CoordRepType p1,CoordRepType p2){
  double diff = p1-p2;
  bool save;
  save = ( (diff<-DIFF_TOLERENCE) || (diff>DIFF_TOLERENCE) );
  return (!save);
}

template <typename TCoordRepType>
unsigned char 
VoronoiDiagram2DGenerator<TCoordRepType>::
Pointonbnd(int VertID){
  PointType currVert = m_OutputVD->GetVertex(VertID);
  if(almostsame(currVert[0],f_pxmin))
    return 1;
  else if(almostsame(currVert[1],f_pymax))
    return 2;
  else if(almostsame(currVert[0],f_pxmax))
    return 3;
  else if(almostsame(currVert[1],f_pymin))
    return 4;
  else
    return 0;
}

template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
ConstructDiagram(void)
{
  EdgeInfoDQ *rawEdges = new EdgeInfoDQ[m_NumberOfSeeds];

  m_OutputVD->Reset();

  EdgeInfo currentPtID;
  int edges = m_OutputVD->EdgeListSize();

  EdgeInfo LRsites; 
  for (int i = 0; i < edges; i++){
    currentPtID = m_OutputVD->GetEdgeEnd(i);
	LRsites = m_OutputVD->GetLine(m_OutputVD->GetEdgeLineID(i));
    rawEdges[LRsites[0]].push_back(currentPtID);
    rawEdges[LRsites[1]].push_back(currentPtID);
    m_OutputVD->AddCellNeighbor(LRsites);	
  }


  PointType corner[4];
  int cornerID[4];

  corner[0][0]=f_pxmin;
  corner[0][1]=f_pymin;
  cornerID[0]=f_nvert;
  f_nvert++;
  m_OutputVD->AddVert(corner[0]);
  corner[1][0]=f_pxmin;
  corner[1][1]=f_pymax;
  cornerID[1]=f_nvert;
  f_nvert++;
  m_OutputVD->AddVert(corner[1]);
  corner[2][0]=f_pxmax;
  corner[2][1]=f_pymax;
  cornerID[2]=f_nvert;
  f_nvert++;
  m_OutputVD->AddVert(corner[2]);
  corner[3][0]=f_pxmax;
  corner[3][1]=f_pymin;
  cornerID[3]=f_nvert;
  f_nvert++;
  m_OutputVD->AddVert(corner[3]);

  std::list<EdgeInfo> buildEdges;
  std::list<EdgeInfo>::iterator BEiter;
  EdgeInfo curr;
  EdgeInfo curr1;
  EdgeInfo curr2;

  unsigned char frontbnd;
  unsigned char backbnd;
  std::vector<unsigned long> cellPoints;
  for(unsigned int i = 0; i < m_NumberOfSeeds; i++){
    buildEdges.clear();
	curr=rawEdges[i].front();
   	rawEdges[i].pop_front();
	buildEdges.push_back(curr);
    EdgeInfo front=curr;
    EdgeInfo back=curr;
	int maxStop=rawEdges[i].size();
	while(!(rawEdges[i].empty())){
      maxStop--;
	  curr=rawEdges[i].front();
  	  rawEdges[i].pop_front();
	  frontbnd=Pointonbnd(front[0]);
	  backbnd=Pointonbnd(back[1]);
	  if(curr[0]==back[1]){
	    buildEdges.push_back(curr);
		back=curr;
	  }
	  else if(curr[1]==front[0]){
	    buildEdges.push_front(curr);
		front=curr;
	  }
	  else if(curr[1]==back[1]){
	    curr1[1]=curr[0];
	    curr1[0]=curr[1];
	    buildEdges.push_back(curr1);
		back=curr1;
	  }
	  else if(curr[0]==front[0]){
	    curr1[0]=curr[1];
	    curr1[1]=curr[0];
	    buildEdges.push_front(curr1);
		front=curr1;
	  }
	  else if( (frontbnd != 0) || (backbnd != 0) )
	  {
          unsigned char cfrontbnd=Pointonbnd(curr[0]);
          unsigned char cbackbnd=Pointonbnd(curr[1]);

		  if((cfrontbnd == backbnd) &&(backbnd)){
		    curr1[0]=back[1];
		    curr1[1]=curr[0];
		    buildEdges.push_back(curr1);
		    buildEdges.push_back(curr);
		    back=curr;
		}
		else if((cbackbnd == frontbnd)&&(frontbnd)){
		  curr1[0]=curr[1];
		  curr1[1]=front[0];
		  buildEdges.push_front(curr1);
		  buildEdges.push_front(curr);
		  front=curr;
		}
		else if((cfrontbnd == frontbnd)&&(frontbnd)){
		  curr1[0]=curr[0];
		  curr1[1]=front[0];
		  buildEdges.push_front(curr1);
		  curr1[0]=curr[1];
		  curr1[1]=curr[0];
		  buildEdges.push_front(curr1);
		  front=curr1;
		}
		else if((cbackbnd == backbnd)&&(backbnd)){
		  curr1[0]=back[1];
		  curr1[1]=curr[1];
		  buildEdges.push_back(curr1);
		  curr1[0]=curr[1];
		  curr1[1]=curr[0];
		  buildEdges.push_back(curr1);
		  back=curr1;
	    }
		else{
          rawEdges[i].push_back(curr);
		}
	  }
	  else{
        rawEdges[i].push_back(curr);
	  }
	}

    curr=buildEdges.front();
	curr1=buildEdges.back();
	if(curr[0] != curr1[1]){
	  frontbnd=Pointonbnd(curr[0]);
	  backbnd=Pointonbnd(curr1[1]);
	  if( (frontbnd!=0) && (backbnd!=0) )
	   {
	    if(frontbnd == backbnd){
	      curr2[0]=curr1[1];
		  curr2[1]=curr[0];
		  buildEdges.push_back(curr2);
		}
	    else if((frontbnd == backbnd+1) || (frontbnd == backbnd-3) ){
	      curr2[0]=cornerID[frontbnd-1];
		  curr2[1]=curr[0];
		  buildEdges.push_front(curr2);
		  curr2[1]=curr2[0];
		  curr2[0]=curr1[1];
		  buildEdges.push_front(curr2);
	    }
	    else if((frontbnd == backbnd-1) || (frontbnd == backbnd+3) ){
	      curr2[0]=cornerID[backbnd-1];
		  curr2[1]=curr[0];
		  buildEdges.push_front(curr2);
		  curr2[1]=curr2[0];
		  curr2[0]=curr1[1];
		  buildEdges.push_front(curr2);
		}
		else{
std::cout<<"Numerical problem 1"<<curr[0]<<" "<<curr1[1]<<std::endl;
		}
	  }
    }

	EdgeInfo pp;

	m_OutputVD->ClearRegion(i);

	for(BEiter = buildEdges.begin(); BEiter != buildEdges.end(); ++BEiter){
	  pp = *BEiter;
      m_OutputVD->VoronoiRegionAddPointId(i,pp[0]);
    }
    m_OutputVD->BuildEdge(i);
  }
  m_OutputVD->InsertCells();
}

/**************************************************************
 **************************************************************
 * Generating Voronoi Diagram using Fortune's Method. (Sweep Line)
 * Infomations are stored in f_VertexList, f_EdgeList and f_LineList;
 */
template <typename TCoordRepType>
bool 
VoronoiDiagram2DGenerator<TCoordRepType>::
right_of(FortuneHalfEdge *el, PointType *p)
{
  FortuneEdge *e = el->m_edge;
  FortuneSite *topsite = e->m_reg[1];

  bool right_of_site = ( ((*p)[0]) > (topsite->m_coord[0]) );
  if (right_of_site && (!(el->m_RorL)) ) return (1);
  if ( (!right_of_site) && (el->m_RorL) ) return (0);
  bool above;
  bool fast;
  if (e->m_a == 1.0){
    double dyp = ((*p)[1]) - (topsite->m_coord[1]);
    double dxp = ((*p)[0]) - (topsite->m_coord[0]);
	fast = 0;
	if( ((!right_of_site) && ((e->m_b)<0.0)) || (right_of_site && ((e->m_b)>=0.0)) )
	{
	  above = ( dyp >= (e->m_b)*dxp );
	  fast = above;
	}
    else
	{
	  above = ( (((*p)[0]) + ((*p)[1])*(e->m_b)) > e->m_c );
	  if(e->m_b < 0.0 ) above = !above;
	  if(!above) fast = 1;
	}
	if(!fast){
	  double dxs = topsite->m_coord[0] - ((e->m_reg[0])->m_coord[0]);
	  above = ( ((e->m_b)*(dxp*dxp-dyp*dyp))<(dxs*dyp*(1.0+2.0*dxp/dxs+(e->m_b)*(e->m_b))) );
	  if((e->m_b) < 0.0) above = !above;
	}
  }
  else { // e->m_b == 1.0 
    double y1 = (e->m_c) - (e->m_a)*((*p)[0]);
	double t1 = ((*p)[1]) -y1;
	double t2 = ((*p)[0]) - topsite->m_coord[0];
	double t3 = y1 - topsite->m_coord[1];
	above = ( (t1*t1) > (t2*t2+t3*t3) );
  }
  return (el->m_RorL? (!above):above);
}


template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
createHalfEdge(FortuneHalfEdge *task, FortuneEdge *e, bool pm)
{
  task->m_edge = e;
  task->m_RorL = pm;
  task->m_next = NULL;
  task->m_vert = NULL;
}


template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
PQshowMin(PointType *answer){
  while( (f_PQHash[f_PQmin].m_next) == NULL){
    f_PQmin += 1;
  }
  (*answer)[0] = f_PQHash[f_PQmin].m_next->m_vert->m_coord[0];
  (*answer)[1] = f_PQHash[f_PQmin].m_next->m_ystar;
}


template <typename TCoordRepType>
void 
VoronoiDiagram2DGenerator<TCoordRepType>::
deletePQ(FortuneHalfEdge *task)
{
  FortuneHalfEdge *last;
  if( (task->m_vert) != NULL){
    last = &(f_PQHash[PQbucket(task)]);
	while ((last->m_next) != task)
	  last = last->m_next;
	last->m_next = (task->m_next);
	f_PQcount--;
	task->m_vert = NULL;
  }
}

template <typename TCoordRepType>
void 
VoronoiDiagram2DGenerator<TCoordRepType>::
deleteEdgeList(FortuneHalfEdge *task)
{
  (task->m_Left)->m_Right = task->m_Right;
  (task->m_Right)->m_Left = task->m_Left;
  task->m_edge = &(f_DELETED);
}


template <typename TCoordRepType>
int 
VoronoiDiagram2DGenerator<TCoordRepType>::
PQbucket(FortuneHalfEdge *task)
{
  int bucket;
  bucket = (int) ((task->m_ystar - f_pymin)/f_deltay * f_PQhashsize);
  if(bucket < 0) bucket = 0;
  if(bucket >= f_PQhashsize) bucket = f_PQhashsize -1;
  if(bucket < f_PQmin) f_PQmin = bucket;
  return(bucket);
}

template <typename TCoordRepType>
void 
VoronoiDiagram2DGenerator<TCoordRepType>::
insertPQ(FortuneHalfEdge *he, FortuneSite *v, double offset)
{

  he->m_vert = v;
  he->m_ystar = (v->m_coord[1]) + offset;
  FortuneHalfEdge *last = &(f_PQHash[PQbucket(he)]);
  FortuneHalfEdge *enext;

  while( ((enext = (last->m_next)) != NULL) && 
         ( ((he->m_ystar) > (enext->m_ystar)) ||
		   ( ((he->m_ystar) == (enext->m_ystar)) && 
		   ( (v->m_coord[0])> (enext->m_vert->m_coord[0]) ))))
	{last = enext;}
  he->m_next = (last->m_next);
  last->m_next = he;
  f_PQcount += 1;
}

template <typename TCoordRepType>
double 
VoronoiDiagram2DGenerator<TCoordRepType>::
dist(FortuneSite *s1,FortuneSite *s2)
{
  double dx = (s1->m_coord[0])-(s2->m_coord[0]);
  double dy = (s1->m_coord[1])-(s2->m_coord[1]);
  return(sqrt(dx*dx+dy*dy));
}

template <typename TCoordRepType>
VoronoiDiagram2DGenerator<TCoordRepType>::FortuneHalfEdge * 
VoronoiDiagram2DGenerator<TCoordRepType>::
ELgethash( int b){
  if( (b<0) || (b>=f_ELhashsize) )
    return (NULL);
  FortuneHalfEdge *he = f_ELHash[b];
  if(he==NULL)
    return (he);
  if(he->m_edge == NULL)
    return (he);
  if((he->m_edge) != (&f_DELETED)) 
    return (he);

  f_ELHash[b] = NULL;

  return (NULL);
}


template <typename TCoordRepType>
VoronoiDiagram2DGenerator<TCoordRepType>::FortuneHalfEdge * 
VoronoiDiagram2DGenerator<TCoordRepType>::
findLeftHE(PointType *p){
  int i;
  int bucket = (int)( (((*p)[0]) - f_pxmin)/f_deltax * f_ELhashsize );
  if(bucket < 0) bucket = 0;
  if(bucket >= f_ELhashsize) bucket = f_ELhashsize - 1;
  FortuneHalfEdge *he = ELgethash(bucket);
  if(he == NULL){
    
	for(i = 1; 1; i++){
	  if( (he=ELgethash(bucket-i)) != NULL) break;
	  if( (he=ELgethash(bucket+i)) != NULL) break;
	}
  }

  if( (he==(&f_ELleftend)) || ((he!=(&f_ELrightend)) && right_of(he,p)) ){
 	do {
	  he = he->m_Right;
	} while ( (he!=(&f_ELrightend)) && (right_of(he,p)) );
	he = he->m_Left;
  }
  else {
    do {
	  he = he->m_Left;
	} while ( (he!=(&f_ELleftend)) && (!right_of(he,p)) );
  }

  if( (bucket>0) && (bucket<f_ELhashsize-1) ){
	f_ELHash[bucket] = he;
  }
  return (he);
}

template <typename TCoordRepType>
VoronoiDiagram2DGenerator<TCoordRepType>::FortuneSite *
VoronoiDiagram2DGenerator<TCoordRepType>::
getRightReg(FortuneHalfEdge *he)
{
  if( (he->m_edge) == NULL )
    return(f_bottomSite);
  else if(he->m_RorL)
    return(he->m_edge->m_reg[0]);
  else 
    return(he->m_edge->m_reg[1]);
}

template <typename TCoordRepType>
VoronoiDiagram2DGenerator<TCoordRepType>::FortuneSite *
VoronoiDiagram2DGenerator<TCoordRepType>::
getLeftReg(FortuneHalfEdge *he)
{
  if( (he->m_edge) == NULL )
    return(f_bottomSite);
  else if(he->m_RorL)
    return(he->m_edge->m_reg[1]);
  else 
    return(he->m_edge->m_reg[0]);
}


template <typename TCoordRepType>
void 
VoronoiDiagram2DGenerator<TCoordRepType>::
insertEdgeList(FortuneHalfEdge *lbase, FortuneHalfEdge *lnew)
{
  lnew->m_Left = lbase;
  lnew->m_Right = lbase->m_Right;
  (lbase->m_Right)->m_Left = lnew;
  lbase->m_Right = lnew;
}

template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
bisect(FortuneEdge *answer, FortuneSite *s1, FortuneSite *s2){
  answer->m_reg[0] = s1;
  answer->m_reg[1] = s2;
  answer->m_ep[0] = NULL;
  answer->m_ep[1] = NULL;

  double dx = (s2->m_coord[0]) - (s1->m_coord[0]);
  double dy = (s2->m_coord[1]) - (s1->m_coord[1]);
  double adx = (dx>0)?dx:-dx;
  double ady = (dy>0)?dy:-dy;

  answer->m_c = (s1->m_coord[0])*dx + (s1->m_coord[1])*dy + (dx*dx+dy*dy)*0.5;
  if(adx > ady){
    answer->m_a = 1.0;
	answer->m_b = dy/dx;
	answer->m_c /=dx;
  }
  else {
    answer->m_a = dx/dy;
	answer->m_b = 1.0;
	answer->m_c /=dy;
  }
  answer->m_edgenbr = f_nedges;
  f_nedges++;
  Point<int, 2> outline;
  outline[0] = answer->m_reg[0]->m_sitenbr;
  outline[1] = answer->m_reg[1]->m_sitenbr;
  m_OutputVD->AddLine(outline);

}


template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
intersect(FortuneSite *newV, FortuneHalfEdge *el1, FortuneHalfEdge *el2)
{
  FortuneEdge *e1 = el1->m_edge;
  FortuneEdge *e2 = el2->m_edge;
  FortuneHalfEdge *saveHE;
  FortuneEdge *saveE;

  if(e1 == NULL){
    newV->m_sitenbr = -1;
	return;
  }
  if(e2 == NULL){
    newV->m_sitenbr = -2;
	return;
  }
  if( (e1->m_reg[1]) == (e2->m_reg[1]) ){
      newV->m_sitenbr = -3;
	  return;
  }
  
  double d = (e1->m_a)*(e2->m_b) - (e1->m_b)*(e2->m_a);
  
  if ( (d>-NUMERIC_TOLERENCE) && (d<NUMERIC_TOLERENCE) )
    {
	newV->m_sitenbr = -4;
	return;
	}
  
  double xmeet = ( (e1->m_c)*(e2->m_b) - (e2->m_c)*(e1->m_b) )/d;
  double ymeet = ( (e2->m_c)*(e1->m_a) - (e1->m_c)*(e2->m_a) )/d;

  if( comp(e1->m_reg[1]->m_coord, e2->m_reg[1]->m_coord) ){
    saveHE = el1;
	saveE = e1;
  }	 
  else {
    saveHE = el2;
	saveE = e2;
  }

  bool right_of_site = (xmeet >= (saveE->m_reg[1]->m_coord[0]) );
  if( (right_of_site && (!(saveHE->m_RorL))) ||
      ( (!right_of_site) && (saveHE->m_RorL)) )
  {
    newV->m_sitenbr = -4;
	return;
  }

  newV->m_coord[0] = xmeet;
  newV->m_coord[1] = ymeet;
  newV->m_sitenbr = -5;
}

template <typename TCoordRepType>
VoronoiDiagram2DGenerator<TCoordRepType>::FortuneHalfEdge *
VoronoiDiagram2DGenerator<TCoordRepType>::
getPQmin(void)
{
  FortuneHalfEdge *curr = f_PQHash[f_PQmin].m_next;
  f_PQHash[f_PQmin].m_next = curr->m_next;
  f_PQcount--;
  return(curr);
}

template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
clip_line(FortuneEdge *task)
{
/* clip line */
  FortuneSite *s1;  
  FortuneSite *s2;  
  double x1,y1,x2,y2;
  if( ((task->m_a)==1.0) && ((task->m_b)>=0.0) ){
    s1 = task->m_ep[1];
    s2 = task->m_ep[0];
  }
  else {
    s2 = task->m_ep[1];
    s1 = task->m_ep[0];
  }

  int id1;
  int id2;
  if( (task->m_a) == 1.0){
	if( (s1 != NULL) && ((s1->m_coord[1]) >f_pymin) ){
	  y1 = s1->m_coord[1];
   	  if(y1 > f_pymax)
	    return;
	  x1 = s1->m_coord[0];
	  id1 = s1->m_sitenbr;
	}
	else{
	  y1 = f_pymin;
  	  x1 = (task->m_c) - (task->m_b)*y1;
	  id1 = -1;
	}

	if ( (s2 != NULL) && ((s2->m_coord[1]) <f_pymax) ){
	  y2 = s2->m_coord[1];
	  if(y2 < f_pymin)
	    return;
	  x2 = s2->m_coord[0];
	  id2 = s2->m_sitenbr;
	}
	else{
	  y2 = f_pymax;
  	  x2 = (task->m_c) - (task->m_b)*y2;
	  id2 = -1;
	}

	if( (x1>f_pxmax) && (x2>f_pxmax) )
	  return;
	if( (x1<f_pxmin) && (x2<f_pxmin) )
	  return;

    if(x1 > f_pxmax){
	  x1 = f_pxmax;
	  y1 = ((task->m_c)-x1)/(task->m_b);
	  id1 = -1;
	}
    if(x1 <f_pxmin){
	  x1 = f_pxmin;
	  y1 = ((task->m_c)-x1)/(task->m_b);
	  id1 = -1;
	}
    if(x2 > f_pxmax){
	  x2 = f_pxmax;
	  y2 = ((task->m_c)-x2)/(task->m_b);
	  id2 = -1;
	}
    if(x2 <f_pxmin){
	  x2 = f_pxmin;
	  y2 = ((task->m_c)-x2)/(task->m_b);
  	  id2 = -1;
	}
  }
  else{
	if( (s1 != NULL) && ((s1->m_coord[0]) >f_pxmin) ){
	  x1 = s1->m_coord[0];
	  if(x1 > f_pxmax)
	    return;
	  y1 = s1->m_coord[1];
	  id1 = s1->m_sitenbr;
	}
	else{
      x1 = f_pxmin;
	  y1 = (task->m_c) - (task->m_a)*x1;
	  id1 = -1;
	}
	x2 = f_pxmax;
	if ( (s2 != NULL) && ((s2->m_coord[0]) <f_pxmax) ){
	  x2 = s2->m_coord[0];
 	  if(x2 < f_pxmin)
	    return;
	  y2 = s2->m_coord[1];
	  id2 = s2->m_sitenbr;
    }
	else{
	  x2 = f_pxmax;
	  y2 = (task->m_c) - (task->m_a)*x2;
	  id2 = -1;
	}
	if( (y1>f_pymax) && (y2>f_pymax) )
	  return;
	if( (y1<f_pymin) && (y2<f_pymin) )
	  return;
    if(y1 > f_pymax){
	  y1 = f_pymax;
	  x1 = ((task->m_c)-y1)/(task->m_a);
	  id1 = -1;
	}
    if(y1 <f_pymin){
	  y1 = f_pymin;
	  x1 = ((task->m_c)-y1)/(task->m_a);
	  id1 = -1;
	}
    if(y2 > f_pymax){
	  y2 = f_pymax;
	  x2 = ((task->m_c)-y2)/(task->m_a);
	  id2 = -1;
	}
    if(y2 <f_pymin){
	  y2 = f_pymin;
	  x2 = ((task->m_c)-y2)/(task->m_a);
	  id2 = -1;
	}
  }

  VoronoiEdge newInfo;
  newInfo.m_Left[0] = x1;
  newInfo.m_Left[1] = y1;
  newInfo.m_Right[0] = x2;
  newInfo.m_Right[1] = y2;
  newInfo.m_LineID = task->m_edgenbr;

  if(id1>-1)
    newInfo.m_LeftID = id1;
  else
  {
    newInfo.m_LeftID = f_nvert;
	f_nvert++;
	PointType newv;
	newv[0]=x1;
	newv[1]=y1;
	m_OutputVD->AddVert(newv);     
  }

  if(id2>-1)
    newInfo.m_RightID = id2;
  else
  {
    newInfo.m_RightID = f_nvert;
	f_nvert++;
	PointType newv;
	newv[0]=x2;
	newv[1]=y2;
	m_OutputVD->AddVert(newv);     
  }
  m_OutputVD->AddEdge(newInfo);
}




template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
makeEndPoint(FortuneEdge *task, bool lr, FortuneSite *ends)
{
  task->m_ep[lr] = ends;
  if ((task->m_ep[1-lr]) == NULL)
    return;
  
  clip_line(task);

}


template <typename TCoordRepType>
void
VoronoiDiagram2DGenerator<TCoordRepType>::
GenerateVDFortune(void)
{

  unsigned int i;

/* Build SeedSites */
  f_SeedSites.resize(m_NumberOfSeeds);
  for(i = 0; i < m_NumberOfSeeds; i++){
    f_SeedSites[i].m_coord = m_Seeds[i];
	f_SeedSites[i].m_sitenbr = i;
  }
/* Initialize Boundary */
  f_pxmax = m_VorBoundary[0];
  f_pymax = m_VorBoundary[1];

  f_deltay = f_pymax - f_pymin;
  f_deltax = f_pxmax - f_pxmin;
  f_sqrtNSites = sqrt((float) (m_NumberOfSeeds + 4));

/* Initialize outputLists */
  f_nedges = 0;
  f_nvert = 0;
  m_OutputVD->LineListClear();
  m_OutputVD->EdgeListClear();
  m_OutputVD->VertexListClear();
  
/* Initialize the Hash Table for Circle Event and Point Event */
  f_PQcount = 0;
  f_PQmin = 0;
  f_PQhashsize = (int)(4 * f_sqrtNSites);
  f_PQHash.resize(f_PQhashsize);
  for (i = 0; i < f_PQhashsize; i++){
    f_PQHash[i].m_next = NULL;
  }
  f_ELhashsize = (int)(2 * f_sqrtNSites);
  f_ELHash.resize(f_ELhashsize);
  for (i = 0; i < f_ELhashsize; i++){
    f_ELHash[i] = NULL;
  }
  createHalfEdge(&(f_ELleftend), NULL, 0);
  createHalfEdge(&(f_ELrightend), NULL, 0);
  f_ELleftend.m_Left = NULL;
  f_ELleftend.m_Right = &(f_ELrightend);
  f_ELrightend.m_Left = &(f_ELleftend);
  f_ELrightend.m_Right = NULL;
  f_ELHash[0] = &(f_ELleftend);
  f_ELHash[f_ELhashsize - 1] = &(f_ELrightend);

  f_bottomSite = &(f_SeedSites[0]);
  FortuneSite *currentSite = &(f_SeedSites[1]);

  PointType currentCircle;
  FortuneHalfEdge *leftHalfEdge;
  FortuneHalfEdge *rightHalfEdge;
  FortuneHalfEdge *left2HalfEdge;
  FortuneHalfEdge *right2HalfEdge;
  FortuneHalfEdge *newHE;
  FortuneSite *findSite;
  FortuneSite *topSite;
  FortuneSite *saveSite;
  FortuneEdge *newEdge;
  FortuneSite *meetSite;
  FortuneSite *newVert;
  bool saveBool;
  
  
  std::vector<FortuneHalfEdge> HEpool;
  std::vector<FortuneEdge> Edgepool;
  std::vector<FortuneSite> Sitepool;
  HEpool.resize(5*m_NumberOfSeeds);
  Edgepool.resize(5*m_NumberOfSeeds);
  Sitepool.resize(5*m_NumberOfSeeds);
  
  int HEid = 0;
  int Edgeid = 0;
  int Siteid = 0;



  i = 2;
  bool ok = 1;
  while(ok){
  	if(f_PQcount != 0){
	  PQshowMin(&currentCircle);
	}
	if( (i <= m_NumberOfSeeds) && ((f_PQcount == 0) || comp(currentSite->m_coord, currentCircle)) ){
	/* Handling Site Event */
      leftHalfEdge = findLeftHE(&(currentSite->m_coord));
      rightHalfEdge = leftHalfEdge->m_Right;

      findSite = getRightReg(leftHalfEdge);

  
      bisect(&(Edgepool[Edgeid]),findSite, currentSite);
      newEdge = &(Edgepool[Edgeid]);
	  Edgeid++;

	  createHalfEdge( &(HEpool[HEid]), newEdge, 0);
	  newHE = &(HEpool[HEid]);
	  HEid++;

	  insertEdgeList(leftHalfEdge, newHE);

      
      intersect(&(Sitepool[Siteid]),leftHalfEdge, newHE);
	  meetSite = &(Sitepool[Siteid]);

      if((meetSite->m_sitenbr) == -5){
	    deletePQ(leftHalfEdge);
		insertPQ(leftHalfEdge, meetSite, dist(meetSite, currentSite));
     	Siteid++;
	  }

	  leftHalfEdge = newHE;
	  createHalfEdge( &(HEpool[HEid]), newEdge, 1);
	  newHE = &(HEpool[HEid]);
	  HEid++;
  
	  insertEdgeList(leftHalfEdge, newHE);

      intersect(&(Sitepool[Siteid]),newHE, rightHalfEdge);
	  meetSite = &(Sitepool[Siteid]);
      if((meetSite->m_sitenbr) == -5){
    	Siteid++;
		insertPQ(newHE, meetSite, dist(meetSite, currentSite));
	  }
      currentSite = &(f_SeedSites[i]);
  	  i++;
    }
	else if(f_PQcount != 0){
	/* Handling Circle Event */

      leftHalfEdge = getPQmin();
	  left2HalfEdge = leftHalfEdge->m_Left;
	  rightHalfEdge = leftHalfEdge->m_Right;
	  right2HalfEdge = rightHalfEdge->m_Right;
	  findSite = getLeftReg(leftHalfEdge);
	  topSite = getRightReg(rightHalfEdge);

	  newVert = leftHalfEdge->m_vert;
	  newVert->m_sitenbr = f_nvert;
	  f_nvert++;
	  m_OutputVD->AddVert(newVert->m_coord);

	  makeEndPoint(leftHalfEdge->m_edge, leftHalfEdge->m_RorL, newVert);
	  makeEndPoint(rightHalfEdge->m_edge, rightHalfEdge->m_RorL, newVert);
      deleteEdgeList(leftHalfEdge);
	  deletePQ(rightHalfEdge);      
	  deleteEdgeList(rightHalfEdge);      

	  saveBool = 0;
	  if( (findSite->m_coord[1]) > (topSite->m_coord[1]) ){
	    saveSite = findSite;
		findSite = topSite;
		topSite = saveSite;
		saveBool = 1;
	  }

      bisect(&(Edgepool[Edgeid]),findSite, topSite);
      newEdge = &(Edgepool[Edgeid]);
	  Edgeid++;
	    
	  createHalfEdge( &(HEpool[HEid]), newEdge, saveBool);
	  newHE = &(HEpool[HEid]);
	  HEid++;

	  insertEdgeList(left2HalfEdge, newHE);
	  makeEndPoint(newEdge, 1-saveBool, newVert);

      intersect(&(Sitepool[Siteid]),left2HalfEdge, newHE);
	  meetSite = &(Sitepool[Siteid]);

      if((meetSite->m_sitenbr) == -5){
  		Siteid++;
	    deletePQ(left2HalfEdge);
		insertPQ(left2HalfEdge, meetSite, dist(meetSite, findSite));
	  }

      intersect(&(Sitepool[Siteid]),newHE, right2HalfEdge);
	  meetSite = &(Sitepool[Siteid]);
      if((meetSite->m_sitenbr) == -5){
	    Siteid++;
		insertPQ(newHE, meetSite, dist(meetSite, findSite));
	  }
	}
	else
	  ok = 0; 
  }

  for( (leftHalfEdge=f_ELleftend.m_Right); (leftHalfEdge != (&f_ELrightend)); (leftHalfEdge=(leftHalfEdge->m_Right)) )
  {
    newEdge = leftHalfEdge->m_edge;
	clip_line(newEdge);
  }
}

}//end namespace

#endif
