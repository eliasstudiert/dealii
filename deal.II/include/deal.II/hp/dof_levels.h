// ---------------------------------------------------------------------
// $Id$
//
// Copyright (C) 2003 - 2013 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the deal.II distribution.
//
// ---------------------------------------------------------------------

#ifndef __deal2__hp_dof_levels_h
#define __deal2__hp_dof_levels_h


#include <deal.II/base/config.h>
#include <deal.II/hp/dof_objects.h>

#include <vector>

DEAL_II_NAMESPACE_OPEN

namespace internal
{
  namespace hp
  {
    /**
     * Store the indices of the degrees of freedom that are located on
     * objects of dimension @p structdim.
     *
     * The things we store here are very similar to what is stored in the
     * internal::DoFHandler::DoFLevel class hierarchy (see there for more
     * information, in particular on the layout of the class hierarchy,
     * and the use of file names). There are two main
     * differences, discussed in the following subsections. In addition to
     * the data already stored by the internal::DoFHandler::DoFLevel
     * classes, we also have to store which finite element each cell
     * uses.
     *
     *
     * <h4>Offset computations</h4>
     *
     * For hp methods, not all cells may use the same finite element, and
     * it is consequently more complicated to determine where the DoF
     * indices for a given line, quad, or hex are stored. As described in
     * the documentation of the internal::DoFHandler::DoFLevel class, we
     * can compute the location of the first line DoF, for example, by
     * calculating the offset as <code>line_index *
     * dof_handler.get_fe().dofs_per_line</code>. This of course doesn't
     * work any more if different lines may have different numbers of
     * degrees of freedom associated with them. Consequently, rather than
     * using this simple multiplication, each of the lines.dofs, quads.dofs,
     * and hexes.dofs arrays has an associated array lines.dof_offsets,
     * quads.dof_offsets, and hexes.dof_offsets. The data corresponding to a
     * line then starts at index <code>line_dof_offsets[line_index]</code>
     * within the <code>line_dofs</code> array.
     *
     *
     * <h4>Multiple data sets per object</h4>
     *
     * If an object corresponds to a cell, the global dof indices of this
     * cell are stored at the location indicated above in sequential
     * order.
     *
     * However, if two adjacent cells use different finite elements, then
     * the face that they share needs to store DoF indices for both
     * involved finite elements. While faces therefore have to have at
     * most two sets of DoF indices, it is easy to see that vertices for
     * example can have as many sets of DoF indices associated with them
     * as there are adjacent cells, and the same holds for lines in 3d.
     *
     * Consequently, for objects that have a lower dimensionality than
     * cells, we have to store a map from the finite element index to the
     * set of DoF indices associated. Since real sets are typically very
     * inefficient to store, and since most of the time we expect the
     * number of individual keys to be small (frequently, adjacent cells
     * will have the same finite element, and only a single entry will
     * exist in the map), what we do is instead to store a linked list. In
     * this format, the first entry starting at position
     * <code>lines.dofs[lines.dof_offsets[line_index]]</code> will denote
     * the finite element index of the set of DoF indices following; after
     * this set, we will store the finite element index of the second set
     * followed by the corresponding DoF indices; and so on. Finally, when
     * all finite element indices adjacent to this object have been
     * covered, we write a -1 to indicate the end of the list.
     *
     * Access to this kind of data, as well as the distinction between
     * cells and objects of lower dimensionality are encoded in the
     * accessor functions, DoFLevel::set_dof_index() and
     * DoFLevel::get_dof_index() They are able to traverse this
     * list and pick out or set a DoF index given the finite element index
     * and its location within the set of DoFs corresponding to this
     * finite element.
     *
     *
     * @ingroup hp
     * @author Wolfgang Bangerth, 1998, 2006, Oliver Kayser-Herold 2003.
     */
    template <int dim>
    class DoFLevel
    {
    public:
      /**
       *  Indices specifying the finite
       *  element of hp::FECollection to use
       *  for the different cells on the current level. The
       *  meaning what a cell is, is
       *  dimension specific, therefore also
       *  the length of this vector depends
       *  on the dimension: in one dimension,
       *  the length of this vector equals
       *  the length of the @p lines vector,
       *  in two dimensions that of the @p
       *  quads vector, etc. The vector stores one element per cell
       *  since the actiev_fe_index is unique for cells.
       */
      std::vector<unsigned int> active_fe_indices;

      /**
       * Store the start index for
       * the degrees of freedom of each
       * object in the @p dofs array.
       *
       * The type we store is then obviously the type the @p dofs array
       * uses for indexing.
       */
      std::vector<std::vector<types::global_dof_index>::size_type> dof_offsets;

      /**
       * Store the global indices of
       * the degrees of freedom. See
       * DoFLevel() for detailed
       * information.
       */
      std::vector<types::global_dof_index> dofs;

      /**
       * Set the global index of
       * the @p local_index-th
       * degree of freedom located
       * on the object with number @p
       * obj_index to the value
       * given by @p global_index. The @p
       * dof_handler argument is
       * used to access the finite
       * element that is to be used
       * to compute the location
       * where this data is stored.
       *
       * The third argument, @p
       * fe_index, denotes which of
       * the finite elements
       * associated with this
       * object we shall
       * access. Refer to the
       * general documentation of
       * the internal::hp::DoFLevel
       * class template for more
       * information.
       */
      template <int dimm, int spacedim>
      void
      set_dof_index (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                     const unsigned int               obj_index,
                     const unsigned int               fe_index,
                     const unsigned int               local_index,
                     const types::global_dof_index    global_index,
                     const unsigned int               obj_level);

      /**
       * Return the global index of
       * the @p local_index-th
       * degree of freedom located
       * on the object with number @p
       * obj_index. The @p
       * dof_handler argument is
       * used to access the finite
       * element that is to be used
       * to compute the location
       * where this data is stored.
       *
       * The third argument, @p
       * fe_index, denotes which of
       * the finite elements
       * associated with this
       * object we shall
       * access. Refer to the
       * general documentation of
       * the internal::hp::DoFLevel
       * class template for more
       * information.
       */
      template <int dimm, int spacedim>
      types::global_dof_index
      get_dof_index (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                     const unsigned int               obj_index,
                     const unsigned int               fe_index,
                     const unsigned int               local_index,
                     const unsigned int               obj_level) const;

      /**
       * Return the number of
       * finite elements that are
       * active on a given
       * object. If this is a cell,
       * the answer is of course
       * one. If it is a face, the
       * answer may be one or two,
       * depending on whether the
       * two adjacent cells use the
       * same finite element or
       * not. If it is an edge in
       * 3d, the possible return
       * value may be one or any
       * other value larger than
       * that.
       *
       * If the object is not part
       * of an active cell, then no
       * degrees of freedom have
       * been distributed and zero
       * is returned.
       */
      template <int dimm, int spacedim>
      unsigned int
      n_active_fe_indices (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                           const unsigned int               obj_index) const;

      /**
       * Return the fe_index of the
       * n-th active finite element
       * on this object.
       */
      template <int dimm, int spacedim>
      types::global_dof_index
      nth_active_fe_index (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                           const unsigned int               obj_level,
                           const unsigned int               obj_index,
                           const unsigned int               n) const;

      /**
       * Check whether a given
       * finite element index is
       * used on the present
       * object or not.
       */
      template <int dimm, int spacedim>
      bool
      fe_index_is_active (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                          const unsigned int               obj_index,
                          const unsigned int               fe_index,
                          const unsigned int               obj_level) const;

      /**
       * Determine an estimate for the
       * memory consumption (in bytes)
       * of this object.
       */
      std::size_t memory_consumption () const;
    };


    // -------------------- template functions --------------------------------

    template <int dim>
    template <int dimm, int spacedim>
    inline
    types::global_dof_index
    DoFLevel<dim>::
    get_dof_index (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                   const unsigned int                obj_index,
                   const unsigned int                fe_index,
                   const unsigned int                local_index,
                   const unsigned int                obj_level) const
    {
      Assert ((fe_index != dealii::hp::DoFHandler<dimm,spacedim>::default_fe_index),
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (fe_index < dof_handler.get_fe().size(),
              ExcIndexRange (fe_index, 0, dof_handler.get_fe().size()));
      Assert (local_index <
              dof_handler.get_fe()[fe_index].template n_dofs_per_object<dim>(),
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index]
                            .template n_dofs_per_object<dim>()));
      Assert (obj_index < dof_offsets.size(),
              ExcIndexRange (obj_index, 0, dof_offsets.size()));

      // make sure we are on an
      // object for which DoFs have
      // been allocated at all
      Assert (dof_offsets[obj_index] != numbers::invalid_dof_index,
              ExcMessage ("You are trying to access degree of freedom "
                          "information for an object on which no such "
                          "information is available"));

      Assert (fe_index == active_fe_indices[obj_index],
	      ExcMessage ("FE index does not match that of the present cell"));
      return dofs[dof_offsets[obj_index]+local_index];
    }



    template <int dim>
    template <int dimm, int spacedim>
    inline
    void
    DoFLevel<dim>::
    set_dof_index (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                   const unsigned int                obj_index,
                   const unsigned int                fe_index,
                   const unsigned int                local_index,
                   const types::global_dof_index     global_index,
                   const unsigned int                obj_level)
    {
      Assert ((fe_index != dealii::hp::DoFHandler<dimm,spacedim>::default_fe_index),
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (fe_index < dof_handler.get_fe().size(),
              ExcIndexRange (fe_index, 0, dof_handler.get_fe().size()));
      Assert (local_index <
              dof_handler.get_fe()[fe_index].template n_dofs_per_object<dim>(),
              ExcIndexRange(local_index, 0,
                            dof_handler.get_fe()[fe_index]
                            .template n_dofs_per_object<dim>()));
      Assert (obj_index < dof_offsets.size(),
              ExcIndexRange (obj_index, 0, dof_offsets.size()));

      // make sure we are on an
      // object for which DoFs have
      // been allocated at all
      Assert (dof_offsets[obj_index] != numbers::invalid_dof_index,
              ExcMessage ("You are trying to access degree of freedom "
                          "information for an object on which no such "
                          "information is available"));

      Assert (fe_index == active_fe_indices[obj_index],
	      ExcMessage ("FE index does not match that of the present cell"));
      dofs[dof_offsets[obj_index]+local_index] = global_index;
    }



    template <int dim>
    template <int dimm, int spacedim>
    inline
    unsigned int
    DoFLevel<dim>::
    n_active_fe_indices (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                         const unsigned int                obj_index) const
    {
      Assert (dim <= dimm, ExcInternalError());
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (obj_index < dof_offsets.size(),
              ExcIndexRange (obj_index, 0, dof_offsets.size()));

      // make sure we are on an
      // object for which DoFs have
      // been allocated at all
      if (dof_offsets[obj_index] == numbers::invalid_dof_index)
        return 0;

      // we are on a cell, so the only set of indices we store is the
      // one for the cell, which is unique
      return 1;
    }



    template <int dim>
    template <int dimm, int spacedim>
    inline
    types::global_dof_index
    DoFLevel<dim>::
    nth_active_fe_index (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                         const unsigned int                obj_level,
                         const unsigned int                obj_index,
                         const unsigned int                n) const
    {
      Assert (dim <= dimm, ExcInternalError());
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (obj_index < dof_offsets.size(),
              ExcIndexRange (obj_index, 0, dof_offsets.size()));

      // make sure we are on an
      // object for which DoFs have
      // been allocated at all
      Assert (dof_offsets[obj_index] != numbers::invalid_dof_index,
              ExcMessage ("You are trying to access degree of freedom "
                          "information for an object on which no such "
                          "information is available"));

      // this is a cell, so there
      // is only a single
      // fe_index
      Assert (n == 0, ExcIndexRange (n, 0, 1));

      return active_fe_indices[obj_index];
    }



    template <int dim>
    template <int dimm, int spacedim>
    inline
    bool
    DoFLevel<dim>::
    fe_index_is_active (const dealii::hp::DoFHandler<dimm,spacedim> &dof_handler,
                        const unsigned int                obj_index,
                        const unsigned int                fe_index,
                        const unsigned int                obj_level) const
    {
      Assert (&dof_handler != 0,
              ExcMessage ("No DoFHandler is specified for this iterator"));
      Assert (&dof_handler.get_fe() != 0,
              ExcMessage ("No finite element collection is associated with "
                          "this DoFHandler"));
      Assert (obj_index < dof_offsets.size(),
              ExcIndexRange (obj_index, 0, static_cast<unsigned int>(dof_offsets.size())));
      Assert ((fe_index != dealii::hp::DoFHandler<dimm,spacedim>::default_fe_index),
              ExcMessage ("You need to specify a FE index when working "
                          "with hp DoFHandlers"));
      Assert (fe_index < dof_handler.get_fe().size(),
              ExcIndexRange (fe_index, 0, dof_handler.get_fe().size()));

      // make sure we are on an
      // object for which DoFs have
      // been allocated at all
      Assert (dof_offsets[obj_index] != numbers::invalid_dof_index,
              ExcMessage ("You are trying to access degree of freedom "
                          "information for an object on which no such "
                          "information is available"));

      Assert (obj_index < active_fe_indices.size(),
	      ExcInternalError());
      return (fe_index == active_fe_indices[obj_index]);
    }

  } // namespace hp

} // namespace internal

DEAL_II_NAMESPACE_CLOSE

#endif
