return {
    groupName = "GetEventListeners",
    cases = {
        {
            name = "Function exists in holytest table",
            when = HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                expect( holytest.GetEventListeners ).to.beA( "function" )
            end
        },
        {
            name = "holytest table doesn't exist",
            when = not HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                expect( holytest ).to.beA( "nil" )
            end
        },
        {
            name = "Report properly",
            when = HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                expect( holytest.GetEventListeners( "hltv_status" ) ).to.equal( 0 )

                gameevent.Listen( "hltv_status" )

                expect( holytest.GetEventListeners( "hltv_status" ) ).to.equal( 1 )
            end
        },
        {
            name = "Report properly when given no event",
            when = HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                expect( holytest.GetEventListeners() ).to.beA( "table" )

                gameevent.Listen( "hltv_status" )

                expect( holytest.GetEventListeners()[ "hltv_title" ] ).to.equal( 1 )
            end
        },
    }
}
