/***********************************************************************************************************************
 *  OpenStudio(R), Copyright (c) 2008-2017, Alliance for Sustainable Energy, LLC. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 *  (1) Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 *  disclaimer.
 *
 *  (2) Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *  following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *  (3) Neither the name of the copyright holder nor the names of any contributors may be used to endorse or promote
 *  products derived from this software without specific prior written permission from the respective party.
 *
 *  (4) Other than as required in clauses (1) and (2), distributions in any form of modifications or other derivative
 *  works may not use the "OpenStudio" trademark, "OS", "os", or any other confusingly similar designation without
 *  specific prior written permission from Alliance for Sustainable Energy, LLC.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, THE UNITED STATES GOVERNMENT, OR ANY CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********************************************************************************************************************/

#include <gtest/gtest.h>
#include "EnergyPlusFixture.hpp"

#include "../ErrorFile.hpp"
#include "../ForwardTranslator.hpp"
#include "../ReverseTranslator.hpp"

#include "../../model/Model.hpp"
#include "../../model/Schedule.hpp"
#include "../../model/Schedule_Impl.hpp"
#include "../../model/ScheduleTypeLimits.hpp"
#include "../../model/ScheduleTypeLimits_Impl.hpp"

#include "../../model/ExternalInterface.hpp"
#include "../../model/ExternalInterface_Impl.hpp"


#include "../../model/Version.hpp"
#include "../../model/Version_Impl.hpp"

#include "../../utilities/geometry/Point3d.hpp"

#include "../../utilities/core/Optional.hpp"
#include "../../utilities/core/Checksum.hpp"
#include "../../utilities/core/UUID.hpp"
#include "../../utilities/core/Logger.hpp"
#include "../../utilities/sql/SqlFile.hpp"
#include "../../utilities/idf/IdfFile.hpp"
#include "../../utilities/idf/IdfObject.hpp"

#include <utilities/idd/OS_ExternalInterface_FieldEnums.hxx>
#include <utilities/idd/ExternalInterface_FieldEnums.hxx>

#include <utilities/idd/IddEnums.hxx>
#include <utilities/idd/IddFactory.hxx>

#include <boost/algorithm/string/predicate.hpp>

#include <QThread>

#include <resources.hxx>

#include <sstream>

#include <vector>

using namespace openstudio::energyplus;
using namespace openstudio::model;
using namespace openstudio;

TEST_F(EnergyPlusFixture, ForwardTranslator_ExternalInterface) {
  Model model;

  ExternalInterface externalinterface = model.getUniqueModelObject<ExternalInterface>();
  EXPECT_EQ("PtolemyServer", externalinterface.nameofExternalInterface());
  EXPECT_FALSE(externalinterface.setNameofExternalInterface("bad value"));
  EXPECT_TRUE(externalinterface.setNameofExternalInterface("FunctionalMockupUnitImport"));
  EXPECT_EQ("FunctionalMockupUnitImport", externalinterface.nameofExternalInterface());

  ForwardTranslator forwardTranslator;
  Workspace workspace = forwardTranslator.translateModel(model);
  EXPECT_EQ(0u, forwardTranslator.errors().size());
  EXPECT_EQ(1u, workspace.getObjectsByType(IddObjectType::ExternalInterface).size());

  WorkspaceObject object = workspace.getObjectsByType(IddObjectType::ExternalInterface)[0];

  ASSERT_TRUE(object.getString(ExternalInterfaceFields::NameofExternalInterface, false));
  EXPECT_EQ("FunctionalMockupUnitImport", object.getString(ExternalInterfaceFields::NameofExternalInterface, false).get());

  model.save(toPath("./ExternalInterface.osm"), true);
  workspace.save(toPath("./ExternalInterface.idf"), true);

}


