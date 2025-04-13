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
            name = "Remove gameevent listeners properly",
            when = HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                expect( holytest.GetEventListeners( "vote_cast" ) ).to.equal( 0 )

                gameevent.Listen( "vote_cast" )

                expect( holytest.GetEventListeners( "vote_cast" ) ).to.equal( 1 )

                holytest.RemoveEventListener( "vote_cast" )

                expect( holytest.GetEventListeners( "vote_cast" ) ).to.equal( 0 )
            end
        },
    }
}
