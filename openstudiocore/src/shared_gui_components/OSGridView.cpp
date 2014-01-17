/**********************************************************************
 *  Copyright (c) 2008-2013, Alliance for Sustainable Energy.  
 *  All rights reserved.
 *  
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *  
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/

#include "OSGridView.hpp"

#include <QAbstractButton>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QStyleOption>

#include "../shared_gui_components/OSCollapsibleView.hpp"

#include <model/Model_impl.hpp>
#include <model/ModelObject_impl.hpp>

#include <utilities/core/Assert.hpp>

namespace openstudio {

OSGridView::OSGridView(std::vector<model::ModelObject> modelObjects, QWidget * parent)
  : QWidget(parent)
{
  //QButtonGroup * buttonGroup = new QButtonGroup();

  //HBoxLayout * buttonLayout = new HBoxLayout();

  //QPushButton * button = 0;

  //for(unsigned i=0; i<m_categories.size(); i++){
  //  button = new QPushButton();
  //  buttonLayout->addWidget(button);
  //}

  m_gridLayout = new QGridLayout();
  m_gridLayout->setSizeConstraint(QLayout::SetMinimumSize);
  m_gridLayout->setAlignment(Qt::AlignTop);
  setLayout(m_gridLayout);

  setContentsMargins(5,5,5,5);
}

void OSGridView::refreshAll()
{
  QLayoutItem *child;
  while((child = m_gridLayout->takeAt(0)) != 0)
  {
      QWidget * widget = child->widget();

      OS_ASSERT(widget);

      delete widget;

      delete child;
  }
}

void OSGridView::refresh(int row, int column)
{
}

} // openstudio

