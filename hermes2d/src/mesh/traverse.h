// This file is part of Hermes2D.
//
// Hermes2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __H2D_TRAVERSE_H
#define __H2D_TRAVERSE_H

/// \brief Determines the position on an element surface (edge in 2D and Face in 3D).
/// \details Used for the retrieval of boundary condition values.
/// \details Same in H2D and H3D.
///
struct SurfPos
{
  int marker;    ///< surface marker (surface = edge in 2D and face in 3D)
  int surf_num;	 ///< local element surface number

  Element *base;                    ///< for internal use
  Space *space, *space_u, *space_v; ///< for internal use

  int v1, v2;    ///< H2D only: edge endpoint vertex id numbers
  double t;      ///< H2D only: position between v1 and v2 in the range [0..1]
  double lo, hi; ///< H2D only: for internal use
};

class Mesh;
class Transformable;
struct State;
struct Rect;


struct UniData
{
  Element* e;
  uint64_t idx;
};


/// Traverse is a multi-mesh traversal utility class. Given N meshes sharing the
/// same base mesh it walks through all (pseudo-)elements of the union of all
/// the N meshes.
///
class HERMES_API Traverse
{
public:

  void begin(int n, Mesh** meshes, Transformable** fn = NULL);
  void finish();

  Element** get_next_state(bool* bnd, SurfPos* surf_pos);
  Element*  get_base() const { return base; }

  UniData** construct_union_mesh(Mesh* unimesh);

private:

  int num;
  Mesh** meshes;
  Transformable** fn;

  State* stack;
  int top, size;

  int id;
  bool tri;
  Element* base;
  int4* sons;
  uint64_t* subs;

  UniData** unidata;
  int udsize;

  State* push_state();
  void set_boundary_info(State* s, bool* bnd, SurfPos* surf_pos);
  void union_recurrent(Rect* cr, Element** e, Rect* er, uint64_t* idx, Element* uni);
  uint64_t init_idx(Rect* cr, Rect* er);

  Mesh* unimesh;

};



#endif
