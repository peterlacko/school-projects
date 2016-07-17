<?php
namespace Poradna;
use Nette;

/**
 *  * Tabulka user
 *   */
class PredmetRepository extends Repository
{
    public function findAll()
    {
	return $this->findAll();
    }

    public function addPredmet($values, $id_fakulty)
    {
	$this->getTable()->insert(array(
	    'Zkratka_predmetu' => $values->zkratka_predmetu,
	    'Nazev_predmetu' => $values->nazev_predmetu,
	    'Kredity'=> $values->kredity,
	    'Ukonceni' => $values->ukonceni,
	    'Garant' => $values->garant,
	    'ID_fakulty' => $id_fakulty
	));
    }

    public function isPredmetInFakulta($nazev_predmetu, $id_fakulty)
    {
	if ($this->getTable()->where(array(
	    'Nazev_predmetu' => $nazev_predmetu,
	    'ID_fakulty' => $id_fakulty))->fetch()) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }

    public function deletePredmet($id) {
	$this->findBy(array('ID_predmetu'=>$id))->delete();
    }

    public function search($phrase)
    {
	return $this->connection->table('predmet')->where("Nazev_predmetu LIKE ?", "%$phrase%");
    }
}
