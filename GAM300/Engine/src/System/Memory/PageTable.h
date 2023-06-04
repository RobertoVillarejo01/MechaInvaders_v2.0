#pragma once
#include <string>
#include "Graphics/Renderable/Renderable.h"
#include "Collisions/Collider.h"
#include "Physics/Rigidbody/Rigidbody.h"

class renderable;
struct StaticCollider;
struct DynamicCollider;
struct Rigidbody;

template <unsigned _id, unsigned _size>
struct m_pair
{
    static const unsigned id = _id;
    static const unsigned size = _size;
};

template <unsigned default_size, typename...>
struct page_map;

template <unsigned default_size>
struct page_map<default_size>
{
    template<unsigned>
    struct get
    {
        static const unsigned size = default_size;
    };
};

template<unsigned default_size, unsigned _id, unsigned _size, typename... rest>
struct page_map<default_size, m_pair<_id, _size>, rest...>
{
    template<unsigned id_to_check>
    struct get
    {
        static const unsigned size =
            (id_to_check == _id) ? _size :
            page_map<default_size, rest...>::template get<id_to_check>::size;
    };
};

typedef page_map<100u,  m_pair<renderable::type_id, 50u>,
                        m_pair<Rigidbody::type_id, 50u>,
                        m_pair<StaticCollider::type_id, 25u>,
                        m_pair<DynamicCollider::type_id, 25u>> mPageTables;