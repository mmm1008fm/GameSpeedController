using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Launcher.Tests
{
    [TestClass]
    public class InjectorTests
    {
        [TestMethod]
        public void InjectDLL_InvalidArguments_ReturnsFalse()
        {
            bool result = Launcher.Injector.InjectDLL(0, "");
            Assert.IsFalse(result);
        }
    }
}
