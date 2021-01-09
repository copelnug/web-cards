#include "Helper.hpp"

#include <boost/property_tree/json_parser.hpp>

namespace
{
	namespace pt = boost::property_tree;
	
	void addStatusAction(pt::ptree& array, const std::string& type, const std::string& label)
	{
		pt::ptree node;
		node.put("type", type);
		node.put("label", label);
		array.push_back(std::make_pair("", node));
	}
}

pt::ptree Serialize::statusNode(const std::string& message)
{
	pt::ptree root;
	root.put(MSG_ENTRY_TYPE, "STATUS");
	root.put("msg", message);
	return root;
}
std::string Serialize::helper(const pt::ptree& msg)
{
	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string Serialize::helperStatus(const std::string& msg)
{
	return helper(statusNode(msg));
}
std::string Serialize::hostStart()
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "ASK_START");

	return Serialize::helper(msg);
}
std::string Serialize::askUsername(const std::string& current)
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "ASK_USERNAME");
	if(!current.empty())
		msg.put("current", current);

	return Serialize::helper(msg);
}
std::string Serialize::illegalChoice()
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "INPUT_INVALID");
	msg.put("msg", TRAD("Choix invalide. Réessayez SVP."));

	return Serialize::helper(msg);
}
std::string Serialize::actionOutOfStep()
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "ERROR");
	msg.put("msg", TRAD("Erreur. Cette action n’est pas celle attendue."));

	return Serialize::helper(msg);
}
std::string Serialize::notPlayerTurn()
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "ERROR");
	msg.put("msg", TRAD("Erreur. Ce n’est pas votre tour."));

	return Serialize::helper(msg);
}
std::string Serialize::waitingStart(const std::string& creator_username)
{
	// TODO Use std::format
	std::ostringstream out;
	out << TRAD("En attente du début de la partie déclaré par ") << creator_username;
	return Serialize::helperStatus(std::move(out).str());
}
std::string Serialize::waitingHost()
{
	// TODO Use std::format
	std::ostringstream out;
	out << TRAD("En attente du créateur de la partie");
	return Serialize::helperStatus(std::move(out).str());
}
std::string Serialize::endGame()
{
	pt::ptree root = Serialize::statusNode("La partie est terminée");
	pt::ptree array;
	addStatusAction(array, "HOME", "Retourner à la page principale");
	root.add_child("actions", std::move(array));
	return Serialize::helper(root);
}