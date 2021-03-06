/*
 * Copyright 2011 Scientific Computation Research Center
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include "apfShape.h"
#include "apfMesh.h"
#include "apfFieldOf.h"
#include "apfElement.h"
#include "apfVectorElement.h"
#include <cassert>

namespace apf {

static const double c = -2.44948974278318;

class Hierarchic : public FieldShape
{
  public:
    Hierarchic() {}
    const char* getName() const { return "Hierarchic"; }
    class Vertex : public EntityShape
    {
      public:
        void getValues(Mesh*, MeshEntity*,
            Vector3 const&, NewArray<double>& N) const
        {
          N.allocate(1);
          N[0] = 1.0;
        }
        void getLocalGradients(Mesh*, MeshEntity*,
            Vector3 const&, NewArray<Vector3>&) const
        {
        }
        int countNodes() const {return 1;}
    };
    class Edge : public EntityShape
    {
      public:
        void getValues(Mesh*, MeshEntity*,
            Vector3 const& xi, NewArray<double>& N) const
        {
          N.allocate(3);
          N[0] = (1.0-xi[0])/2.0;
          N[1] = (1.0+xi[0])/2.0;
          N[2] = c*N[0]*N[1];
        }
        void getLocalGradients(Mesh*, MeshEntity*,
            Vector3 const& xi, NewArray<Vector3>& dN) const
        {
          dN.allocate(3);
          dN[0] = Vector3(-0.5, 0.0, 0.0);
          dN[1] = Vector3( 0.5, 0.0, 0.0);
          dN[2] = Vector3(-0.5*c*xi[0], 0.0, 0.0);
        }
        int countNodes() const {return 3;}
    };
    class Triangle : public EntityShape
    {
      public:
        void getValues(Mesh*, MeshEntity*,
            Vector3 const& xi, NewArray<double>& N) const
        {
          N.allocate(6);
          N[0] = 1.0-xi[0]-xi[1];
          N[1] = xi[0];
          N[2] = xi[1];
          N[3] = c*N[0]*N[1];
          N[4] = c*N[1]*N[2];
          N[5] = c*N[2]*N[0];
        }
        void getLocalGradients(Mesh*, MeshEntity*,
            Vector3 const& xi, NewArray<Vector3>& dN) const
        {
          dN.allocate(6);
          dN[0] = Vector3(-1.0, -1.0, 0.0);
          dN[1] = Vector3( 1.0,  0.0, 0.0);
          dN[2] = Vector3( 0.0,  1.0, 0.0);
          dN[3] = Vector3( 1.0-2.0*xi[0]-xi[1],  -xi[0], 0.0) * c;
          dN[4] = Vector3( xi[1], xi[0], 0.0) * c;
          dN[5] = Vector3( -xi[1],  1.0-xi[0]-2.0*xi[1], 0.0) * c;
        }
        int countNodes() const {return 6;}
    };
    class Tetrahedron : public EntityShape
    {
      public:
        void getValues(Mesh*, MeshEntity*,
            Vector3 const& xi, NewArray<double>& N) const
        {
          N.allocate(10);
          N[0] = 1.0-xi[0]-xi[1]-xi[2];
          N[1] = xi[0];
          N[2] = xi[1];
          N[3] = xi[2];
          N[4] = c*N[0]*N[1];
          N[5] = c*N[1]*N[2];
          N[6] = c*N[2]*N[0];
          N[7] = c*N[0]*N[3];
          N[8] = c*N[1]*N[3];
          N[9] = c*N[2]*N[3];
        }
        void getLocalGradients(Mesh*, MeshEntity*,
            Vector3 const& xi, NewArray<Vector3>& dN) const
        {
          dN.allocate(10);
          dN[0] = Vector3(-1.0, -1.0, -1.0);
          dN[1] = Vector3( 1.0,  0.0,  0.0);
          dN[2] = Vector3( 0.0,  1.0,  0.0);
          dN[3] = Vector3( 0.0,  0.0,  1.0);
          dN[4] = Vector3( 1.0-2.0*xi[0]-xi[1]-xi[2], -xi[0], -xi[0] ) * c;
          dN[5] = Vector3( xi[1], xi[0], 0.0 ) * c;
          dN[6] = Vector3( -xi[1], 1.0-xi[0]-2.0*xi[1]-xi[2], -xi[1] ) * c;
          dN[7] = Vector3( -xi[2], -xi[2], 1.0-xi[0]-xi[1]-2.0*xi[2] ) * c;
          dN[8] = Vector3( xi[2], 0.0, xi[0] ) * c;
          dN[9] = Vector3( 0.0, xi[2], xi[1] ) * c;
        }
        int countNodes() const {return 10;}
    };
    EntityShape* getEntityShape(int type)
    {
      static Vertex vertex;
      static Edge edge;
      static Triangle triangle;
      static Tetrahedron tet;
      static EntityShape* shapes[Mesh::TYPES] =
      {&vertex,   //vertex
       &edge,     //edge
       &triangle, //triangle
       NULL,     //quad
       &tet,      //tet
       NULL,      //hex
       NULL,      //prism
       NULL};     //pyramid
      return shapes[type];
    }
    void getNodeXi(int, int, Vector3& xi)
    {
      xi = Vector3(0,0,0);
    }
    bool hasNodesIn(int dimension)
    {
      if ( (dimension == 0) || (dimension == 1) )
        return true;
      else
        return false;
    }
    int countNodesOn(int type)
    {
      if ( (type == Mesh::VERTEX) || (type == Mesh::EDGE) )
        return 1;
      else
        return 0;
    }
    int getOrder() {return 2;}
};

FieldShape* getHierarchic(int o)
{
  static Hierarchic q;
  if (o == 1)
    return getLagrange(o);
  else if (o == 2)
    return &q;
  return NULL;
}

template <class T>
class Projector : public FieldOp
{
  public:
    Projector(Field* a, Field* b)
    {
      to = static_cast<FieldOf<T>*>(a);
      from = static_cast<FieldOf<T>*>(b);
      mesh = to->getMesh();
      meshElem = 0;
      fromElem = 0;
      int n = to->countComponents();
      data.allocate(n);
      for (int i=0; i < n; ++i)
        data[i] = 0.0;
    }
    bool inEntity(MeshEntity* e)
    {
      meshElem = createMeshElement(mesh, e);
      fromElem = static_cast<ElementOf<T>*>(createElement(from, meshElem));
      return true;
    }
    void atNode(int n)
    {
      int nt = to->countNodesOn(meshElem->getEntity());
      int nf = from->countNodesOn(meshElem->getEntity());
      if ( (nf == 0) || ((nf - nt) < 0))
        setComponents(to, meshElem->getEntity(), n, &data[0]);
      else {
        Vector3 xi;
        to->getShape()->getNodeXi(fromElem->getType(),n,xi);
        T value[1];
        value[0] = fromElem->getValue(xi);
        to->setNodeValue(fromElem->getEntity(),n,value);
      }
    }
    void outEntity()
    {
      destroyMeshElement(meshElem);
      destroyElement(fromElem);
    }
    void run()
    {
      apply(to);
    }
    FieldOf<T>* to;
    FieldOf<T>* from;
    Mesh* mesh;
    VectorElement* meshElem;
    ElementOf<T>* fromElem;
    apf::NewArray<double> data;
};

void projectHierarchicField(Field* to, Field* from)
{
  int ttype = to->getValueType();
  int ftype = from->getValueType();
  assert(ttype == ftype);
  if (ttype == SCALAR) {
    Projector<double> projector(to,from);
    projector.run();
  }
  else if (ttype == VECTOR) {
    Projector<Vector3> projector(to,from);
    projector.run();
  }
  else if (ttype == MATRIX) {
    Projector<Matrix3x3> projector(to,from);
    projector.run();
  }
  else
    fail("projectHierarchicField: unsupported value type");
}

}
